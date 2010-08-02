/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2004 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Wez Furlong  <wez@php.net>                                   |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ffi.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_object_handlers.h"

#include "php_ffi_internal.h"

static void *php_ffi_parser_malloc_proc(size_t size)
{
	return emalloc(size);
}

static void php_ffi_parser_free_proc(void *mem)
{
	efree(mem);
}

static struct {
	const char *tokenname;
	int tokenlen;
	int tokval;
	int minor;
} keywords[] = {
#define MKKEYWORD(label, value)	{ label, sizeof(label)-1, value, 0 }
	MKKEYWORD("struct", PHP_FFI_TOK_STRUCT),
	MKKEYWORD("typedef", PHP_FFI_TOK_TYPEDEF),
	MKKEYWORD("callback", PHP_FFI_TOK_CALLBACK),
#define MKTYPETOK(label, value)	{ label, sizeof(label)-1, PHP_FFI_TOK_INTRINSIC, value }
	MKTYPETOK("void", FFI_TYPE_VOID),
	MKTYPETOK("int", FFI_TYPE_INT ),
	MKTYPETOK("float", FFI_TYPE_FLOAT ),
	MKTYPETOK("double", FFI_TYPE_DOUBLE ),
	MKTYPETOK("uint8", FFI_TYPE_UINT8 ),
	MKTYPETOK("sint8", FFI_TYPE_SINT8 ),
	MKTYPETOK("char",  FFI_TYPE_SINT8 ),
	MKTYPETOK("uint16", FFI_TYPE_UINT16 ),
	MKTYPETOK("sint16", FFI_TYPE_SINT16 ),
	MKTYPETOK("uint32", FFI_TYPE_UINT32 ),
	MKTYPETOK("sint32", FFI_TYPE_SINT32 ),
	MKTYPETOK("uint64", FFI_TYPE_UINT64 ),
	MKTYPETOK("sint64", FFI_TYPE_SINT64 ),

#if SIZEOF_SHORT == 2
	MKTYPETOK("short", FFI_TYPE_SINT16),
#elif SIZEOF_SHORT == 4
	MKTYPETOK("short", FFI_TYPE_SINT32),
#elif SIZEOF_SHORT == 8
	MKTYPETOK("short", FFI_TYPE_SINT64),
#endif

#if SIZEOF_LONG == 2
	MKTYPETOK("long", FFI_TYPE_SINT16),
#elif SIZEOF_LONG == 4
	MKTYPETOK("long", FFI_TYPE_SINT32),
#elif SIZEOF_LONG == 8
	MKTYPETOK("long", FFI_TYPE_SINT64),
#endif

#ifdef PHP_WIN32
	MKTYPETOK("DWORD", FFI_TYPE_UINT32),
	MKTYPETOK("WORD", FFI_TYPE_UINT16),
	MKTYPETOK("BYTE", FFI_TYPE_UINT8),
	MKTYPETOK("LONG", FFI_TYPE_SINT32),
	MKTYPETOK("ULONG", FFI_TYPE_UINT32),
	MKTYPETOK("SHORT", FFI_TYPE_SINT16),
	MKTYPETOK("CHAR", FFI_TYPE_SINT8),
	MKTYPETOK("INT", FFI_TYPE_INT),
	MKTYPETOK("VOID", FFI_TYPE_VOID),
#endif
	
	{ NULL, 0, 0 }
};

void *php_ffi_parserAlloc(void *(*mallocProc)(size_t));
void php_ffi_parserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
);

static int parse_defs(php_ffi_context *the_ctx, char *proto_text)
{
	void *p;
	int tok;
	php_ffi_tokentype value;
	char *c, *tok_start;
	int ret = 1, i;
	struct php_ffi_def_context ctx = {0};

	ctx.ctx = the_ctx;

	p = php_ffi_parserAlloc(php_ffi_parser_malloc_proc);

	c = proto_text;

	while (*c) {
		while (isspace(*c)) {
			c++;
		}
		if (*c == '\0') {
			break;
		}
		tok_start = c;

		switch (*c) {
			case '\'':
				c++;
				tok_start = c;
				while (*c != '\'') {
					c++;
				}
				value.ident.val = tok_start;
				tok = PHP_FFI_TOK_STRING;
				value.ident.len = c - tok_start;
				break;

			case ';': tok = PHP_FFI_TOK_SEMI; 	break;
			case '(': tok = PHP_FFI_TOK_LPAREN;	break;
			case ')': tok = PHP_FFI_TOK_RPAREN;	break;
			case '{': tok = PHP_FFI_TOK_LBRACE;	break;
			case '}': tok = PHP_FFI_TOK_RBRACE; break;
			case '[': tok = PHP_FFI_TOK_LSQUARE; break;
			case ']': tok = PHP_FFI_TOK_RSQUARE; break;
			case '*': tok = PHP_FFI_TOK_ASTERISK; break;
			case '=': tok = PHP_FFI_TOK_EQUALS; break;
			case ',': tok = PHP_FFI_TOK_COMMA; break;
			default:
				if (isalpha(*c) || *c == '_') {
		  			while (isalnum(*c) || *c == '_') {
  						c++;
					}
					value.ident.val = tok_start;
					tok = PHP_FFI_TOK_IDENT;
					value.ident.len = c - tok_start;
					c--;

					/* look up keywords */
					for (i = 0; keywords[i].tokenname; i++) {
						if (value.ident.len == keywords[i].tokenlen &&
								memcmp(value.ident.val, keywords[i].tokenname, value.ident.len) == 0) {
							tok = keywords[i].tokval;
	
							switch (tok) {
								case PHP_FFI_TOK_INTRINSIC:
									value.type.intrinsic_type = keywords[i].minor;
									break;
							}
							
							break;
						}
					}
				} else {
					ret = 0;
					goto out;
				}
		}

#if ZEND_DEBUG
		printf("scanned: -> '%s'\n", php_ffi_get_token_string(tok, value));
#endif

		php_ffi_parser(p, tok, value, &ctx);
		if (ctx.failed) {
			break;
		}
		c++;
	}

	if (!ctx.failed) {
		php_ffi_parser(p, 0, value, &ctx);
	}

out:
	php_ffi_parserFree(p, php_ffi_parser_free_proc);

	if (ctx.args) {
		efree(ctx.args);
	}
	
	return ret;
}

static PHP_FUNCTION(php_ffi_context_create_instance)
{
	zval *object = getThis();
	php_ffi_context *obj;
	char *funcdefs;
	long funcdefslen;

	obj = CTX_FETCH(object);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
			&funcdefs, &funcdefslen)) {
		ZVAL_NULL(object);
		return;
	}
	
	/* now parse the function definitions */
	parse_defs(obj, funcdefs);
}

zend_function_entry php_ffi_context_funcs[] = {
	{ "__construct", PHP_FN(php_ffi_context_create_instance), NULL },
	{ NULL, NULL, NULL }
};


void php_ffi_type_dtor(void *pDest)
{
	int i;
	php_ffi_type_def *def = (php_ffi_type_def*)pDest;

	if (def->struct_name) {
		efree(def->struct_name);
	}
	if (def->ffi_t.elements) {
		efree(def->ffi_t.elements);
	}
	if (def->field_names) {
		for (i = 0; i < def->nfields; i++) {
			efree(def->field_names[i]);
		}
		efree(def->field_names);
	}
	if (def->field_types) {
		efree(def->field_types);
	}
}

void php_ffi_lib_dtor(void *pDest)
{
	php_ffi_library *lib = (php_ffi_library*)pDest;

	if (lib->handle) {
		DL_UNLOAD(lib->handle);
	}
	if (lib->libname) {
		efree(lib->libname);
	}
}

void php_ffi_func_dtor(void *pDest)
{
	php_ffi_function *func = (php_ffi_function*)pDest;

	if (func->cif_arg_types) {
		efree(func->cif_arg_types);
	}
	if (func->arg_types) {
		efree(func->arg_types);
	}
	if (func->arg_info) {
		efree(func->arg_info);
	}
}

static zval *php_ffi_property_read(zval *object, zval *member, int type TSRMLS_DC)
{
	zval *return_value;
	php_ffi_context *obj;

	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);

	obj = CTX_FETCH(object);

	PHP_FFI_THROW("ffi_libraries have no properties");

	return return_value;
}

static void php_ffi_property_write(zval *object, zval *member, zval *value TSRMLS_DC)
{
	PHP_FFI_THROW("ffi_libraries have no properties");
}

static zval *php_ffi_read_dimension(zval *object, zval *offset, int type TSRMLS_DC)
{
	zval *return_value;

	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);

	PHP_FFI_THROW("ffi_libraries have no dimensions");
	return return_value;
}

static void php_ffi_write_dimension(zval *object, zval *offset, zval *value TSRMLS_DC)
{
	PHP_FFI_THROW("ffi_libraries have no dimensions");
}

static void php_ffi_object_set(zval **property, zval *value TSRMLS_DC)
{
	/* Not yet implemented in the engine */
}

static zval *php_ffi_object_get(zval *property TSRMLS_DC)
{
	/* Not yet implemented in the engine */
	return NULL;
}

static int php_ffi_property_exists(zval *object, zval *member, int check_empty TSRMLS_DC)
{
	return 0;
}

static void php_ffi_property_delete(zval *object, zval *member TSRMLS_DC)
{
	PHP_FFI_THROW("Cannot delete properties from an ffi_library");
}

static void php_ffi_dimension_delete(zval *object, zval *offset TSRMLS_DC)
{
	PHP_FFI_THROW("Cannot delete properties from an ffi_library");
}

static int php_ffi_dimension_exists(zval *object, zval *member, int check_empty TSRMLS_DC)
{
	return 0;
}

static HashTable *php_ffi_properties_get(zval *object TSRMLS_DC)
{
	return NULL;
}

static php_ffi_function *bind_func(php_ffi_context *ctx, char *name, int len TSRMLS_DC)
{
	php_ffi_function *func;

	if (FAILURE == zend_hash_find(&ctx->functions, name, len, (void**)&func)) {
		return NULL;
	}

	/* bind to the library and the function address */
	if (func->lib->handle == NULL) {
		func->lib->handle = DL_LOAD(func->lib->libname);

		if (func->lib->handle == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not load library \"%s\"", func->lib->libname);
			return NULL;
		}
	}

	if (func->func_addr == NULL) {
#if defined(PHP_WIN32)
# define php_ffi_get_sym(handle, name)	GetProcAddress(handle, name)
#elif HAVE_LIBDL
# define php_ffi_get_sym(handle, name)	dlsym(handle, name)
#elif defined(HAVE_MACH_O_DYLD_H)
# define php_ffi_get_sym(handle, name)	zend_mh_bundle_symbol(handle, name)
#else
# error You lose
#endif
#if defined(DLSYM_NEEDS_UNDERSCORE) || defined(HAVE_MACH_O_DYLD_H)
		char symbolname[256];

		snprintf(symbolname, sizeof(symbolname), "_%s", name);
		name = symbolname;
#endif
		func->func_addr = php_ffi_get_sym(func->lib->handle, name);
		if (func->func_addr == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "function \"%s\" was not found in library %s",
					name, func->lib->libname);
			return NULL;
		}
	}
	return func;
}

static union _zend_function *php_ffi_method_get(
#if PHP_API_VERSION >= 20041225
	zval **object_pp,
#else
	zval *object,
#endif
	char *name, int len TSRMLS_DC)
{
	zend_internal_function *f;
	php_ffi_context *obj;
	php_ffi_function *func;
#if PHP_API_VERSION >= 20041225
	zval *object = *object_pp;
#endif

	obj = CTX_FETCH(object);

	func = bind_func(obj, name, len TSRMLS_CC);

	if (func == NULL) {
		if (obj->ce != php_ffi_context_class_entry) {
#if 0
			zend_internal_function *call_user_call = emalloc(sizeof(zend_internal_function));
		        call_user_call->type = ZEND_INTERNAL_FUNCTION;
		        call_user_call->handler = zend_std_call_user_call;
		        call_user_call->arg_info = NULL;
		        call_user_call->num_args = 0;
		        call_user_call->scope = obj->ce;
		        call_user_call->fn_flags = 0;
		        call_user_call->function_name = estrndup(name, len);

		        return (union _zend_function *)call_user_call;
#else
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "this doesn't work right now (unable to call %s)", name);
#endif
		}

		return NULL;
	}

	f = emalloc(sizeof(zend_internal_function));
	f->type = ZEND_OVERLOADED_FUNCTION_TEMPORARY;
	f->num_args = func->nargs;
	f->scope = php_ffi_context_class_entry;
	f->fn_flags = 0;
	f->function_name = estrdup(name);
	f->arg_info = func->arg_info;

	return (union _zend_function*)f;
}

static int php_ffi_call_method(char *method, INTERNAL_FUNCTION_PARAMETERS)
{
	zval **args = NULL;
	php_ffi_context *obj;
	int nargs, i;
	int ret = FAILURE;
	php_ffi_function *func;
	void **values = NULL;
	int *need_free = NULL;
	char *return_value_buf = NULL;

	obj = CTX_FETCH(getThis());

	func = bind_func(obj, method, strlen(method) TSRMLS_CC);

	if (func == NULL) {
		return FAILURE;
	}

	nargs = ZEND_NUM_ARGS();

	if (nargs != func->nargs) {
		PHP_FFI_THROW("incorrect number of parameters");
		return FAILURE;
	}

	if (nargs) {
		args = (zval **)safe_emalloc(sizeof(zval *), nargs, 0);
		zend_get_parameters_array(ht, nargs, args);

		values = (void**)safe_emalloc(sizeof(void*), nargs, 0);
		need_free = (int*)safe_emalloc(sizeof(int), nargs, 0);
	}

/*	DebugBreak(); */
	for (i = 0; i < nargs; i++) {
		need_free[i] = 1; /* we want mem allocated if needed */
		if (!php_ffi_zval_to_native(&values[i], &need_free[i], args[i], &func->arg_types[i] TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not map parameter %d", i);
		}
	}

	if (return_value_used) {
		if (func->ret_type.type && func->ret_type.type->type != FFI_TYPE_VOID) {
			return_value_buf = emalloc(func->ret_type.type->size);
		} else if (func->ret_type.ptr_levels) {
			return_value_buf = emalloc(sizeof(void*));
		} else if (func->ret_type.tdef) {
			return_value_buf = emalloc(func->ret_type.tdef->total_size);
		}
	}
	ffi_call(&func->cif, func->func_addr, return_value_buf, values);

	/* pull back the return value */
	if (return_value_buf && !php_ffi_native_to_zval(return_value_buf, &func->ret_type, return_value TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not map return value");
	}

	if (args) {
		efree(args);
	}

	for (i = 0; i < nargs; i++) {
		if (need_free[i]) {
			efree(values[i]);
		}
	}

	if (values) {
		efree(values);
	}

	if (return_value_buf) {
		efree(return_value_buf);
	}

	if (need_free) {
		efree(need_free);
	}

	return ret;
}

static union _zend_function *php_ffi_constructor_get(zval *object TSRMLS_DC)
{
	php_ffi_context *obj;

	obj = CTX_FETCH(object);
	
	if (obj->ce != php_ffi_context_class_entry) {
		return obj->ce->constructor;
	} else {
		static zend_internal_function f;

		f.function_name = php_ffi_context_class_entry->name;
		f.scope = php_ffi_context_class_entry;
		f.arg_info = NULL;
		f.num_args = 0;
		f.fn_flags = 0;

		f.type = ZEND_INTERNAL_FUNCTION;
		f.handler = ZEND_FN(php_ffi_context_create_instance);
	
		return (union _zend_function*)&f;
	}
}

static zend_class_entry *php_ffi_class_entry_get(zval *object TSRMLS_DC)
{
	php_ffi_context *obj;
	obj = CTX_FETCH(object);

	return obj->ce;
}

static int php_ffi_class_name_get(zval *object, char **class_name, zend_uint *class_name_len, int parent TSRMLS_DC)
{
	php_ffi_context *obj;
	obj = CTX_FETCH(object);

	*class_name = estrndup(obj->ce->name, obj->ce->name_length);
	*class_name_len = obj->ce->name_length;

	return 0;
}

/* This compares two variants for equality */
static int php_ffi_objects_compare(zval *object1, zval *object2 TSRMLS_DC)
{
	return -1;
}

static int php_ffi_object_cast(zval *readobj, zval *writeobj, int type TSRMLS_DC)
{
	return FAILURE;
}

static zend_object_handlers php_ffi_object_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,
	php_ffi_property_read,
	php_ffi_property_write,
	php_ffi_read_dimension,
	php_ffi_write_dimension,
	NULL,
	NULL, /* php_ffi_object_get, */
	NULL, /* php_ffi_object_set, */
	php_ffi_property_exists,
	php_ffi_property_delete,
	php_ffi_dimension_exists,
	php_ffi_dimension_delete,
	php_ffi_properties_get,
	php_ffi_method_get,
	php_ffi_call_method,
	php_ffi_constructor_get,
	php_ffi_class_entry_get,
	php_ffi_class_name_get,
	php_ffi_objects_compare,
	php_ffi_object_cast,
	NULL, /* count */
};

void php_ffi_context_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	php_ffi_context *obj = (php_ffi_context*)object;

	zend_hash_destroy(&obj->functions);
	zend_hash_destroy(&obj->libraries);
	zend_hash_destroy(&obj->types);

	efree(obj);
}

void php_ffi_object_clone(void *object, void **clone_ptr TSRMLS_DC)
{
	php_ffi_library *cloneobj, *origobject;

	origobject = (php_ffi_library*)object;
	cloneobj = (php_ffi_library*)emalloc(sizeof(php_ffi_library));
	
	memcpy(cloneobj, origobject, sizeof(*cloneobj));

	/* TODO: some refcount stuff ? */	

	*clone_ptr = cloneobj;
}

zend_object_value php_ffi_context_object_new(zend_class_entry *ce TSRMLS_DC)
{
	php_ffi_context *obj;
	zend_object_value retval;

	obj = emalloc(sizeof(*obj));
	memset(obj, 0, sizeof(*obj));

	zend_hash_init(&obj->functions, 2, NULL, php_ffi_func_dtor, 0);
	zend_hash_init(&obj->libraries, 2, NULL, php_ffi_lib_dtor, 0);
	zend_hash_init(&obj->types, 2, NULL, php_ffi_type_dtor, 0);
	obj->ce = ce;
	
	retval.handle = zend_objects_store_put(obj, php_ffi_context_dtor, NULL, php_ffi_object_clone TSRMLS_CC);
	retval.handlers = &php_ffi_object_handlers;

	return retval;
}
