%name php_ffi_parser
%extra_argument { struct php_ffi_def_context *ctx }
%token_type { php_ffi_tokentype }
%token_prefix PHP_FFI_TOK_
%parse_failure { ctx->failed = 1; printf("FAIL: parser failed - %d errors\n", ctx->errors); }
%syntax_error { ctx->errors++; printf("SYNTAX: entering error recovery near token %s\n", php_ffi_get_token_string(yymajor, TOKEN)); }
%include {
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
   | Author: Wez Furlong  <wez@thebrainroom.com>                          |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ffi.h"
#include "php_ffi_internal.h"

#ifdef ZTS
# define CTX_TSRMLS_FETCH()	void *tsrm_ls = ctx->tsrm_ls
#else
# define CTX_TSRMLS_FETCH()
#endif

#define IDENT_EQUALS(str, v)	(v.len == sizeof(str)-1 && memcmp(str, v.val, v.len) == 0)

static const char *get_ident_string(php_ffi_ident id)
{
	static char idbuf[256];

	memcpy(idbuf, id.val, id.len);
	idbuf[id.len] = '\0';

	return idbuf;
}
	
const char *php_ffi_get_token_string(int major, php_ffi_tokentype t)
{
	static char tokbuf[512];
	char *c;

	switch (major) {
		case PHP_FFI_TOK_INTRINSIC:
			switch (t.type.intrinsic_type) {
				case FFI_TYPE_VOID:	c = "void"; break;
				case FFI_TYPE_INT: c= "int"; break;
				case FFI_TYPE_FLOAT: c = "float"; break;
				case FFI_TYPE_DOUBLE: c = "double"; break;
				case FFI_TYPE_LONGDOUBLE: c = "long double"; break;
				case FFI_TYPE_UINT8: c = "uint8"; break;
				case FFI_TYPE_SINT8: c = "sint8"; break;
				case FFI_TYPE_UINT16: c = "uint16"; break;
				case FFI_TYPE_SINT16: c = "sint16"; break;
				case FFI_TYPE_UINT32: c = "uint32"; break;
				case FFI_TYPE_SINT32: c = "sint32"; break;
				case FFI_TYPE_UINT64: c = "uint64"; break;
				case FFI_TYPE_SINT64: c = "sint64"; break;
				case FFI_TYPE_STRUCT: c = "struct"; break;
				case FFI_TYPE_POINTER: c = "pointer"; break;
				default:
					c = "unknown/invalid";
			}
			sprintf(tokbuf, "intrinsic type %d %s", t.type.intrinsic_type, c);
			break;
		case PHP_FFI_TOK_STRING:
			sprintf(tokbuf, "string: %s", get_ident_string(t.ident));
			break;

		case PHP_FFI_TOK_IDENT:
			sprintf(tokbuf, "identifier: %s", get_ident_string(t.ident));
			break;
		default:
			sprintf(tokbuf, "token: %s", php_ffi_parserTokenName(major));
	}
	return tokbuf;
}

static void add_arg(struct php_ffi_def_context *ctx, php_ffi_type_ref type, php_ffi_ident ident)
{
	php_ffi_arg_def *def;
	
	/* grow the args array if needed */
	if (ctx->n_args == ctx->arg_size) {
		ctx->arg_size += 8;
		ctx->args = erealloc(ctx->args, ctx->arg_size * sizeof(php_ffi_arg_def));
	}

	def = &ctx->args[ctx->n_args++];

	def->type = type;
	def->name = ident;
}

static ffi_type *resolve_type(struct php_ffi_def_context *ctx, php_ffi_type_ref type,
	struct php_ffi_field_def *fdef TSRMLS_DC)
{
	ffi_type *ft = NULL;
	php_ffi_type_def *tdef = NULL;

	if (type.ptr_levels > 0 && fdef == NULL) {
		return &ffi_type_pointer;
	}

	switch (type.intrinsic_type) {
		case FFI_TYPE_VOID:			ft = &ffi_type_void; break;
		case FFI_TYPE_INT:			ft = &ffi_type_sint; break;
		case FFI_TYPE_FLOAT:		ft = &ffi_type_float; break;
		case FFI_TYPE_DOUBLE:		ft = &ffi_type_double; break;
		case FFI_TYPE_LONGDOUBLE:	ft = &ffi_type_longdouble; break;
		case FFI_TYPE_UINT8:		ft = &ffi_type_uint8; break;
		case FFI_TYPE_SINT8:		ft = &ffi_type_sint8; break;
		case FFI_TYPE_UINT16:		ft = &ffi_type_uint16; break;
		case FFI_TYPE_SINT16:		ft = &ffi_type_sint16; break;
		case FFI_TYPE_UINT32:		ft = &ffi_type_uint32; break;
		case FFI_TYPE_SINT32:		ft = &ffi_type_sint32; break;
		case FFI_TYPE_UINT64:		ft = &ffi_type_uint64; break;
		case FFI_TYPE_SINT64:		ft = &ffi_type_sint64; break;
		case FFI_TYPE_POINTER:		ft = &ffi_type_pointer; break;
		case FFI_TYPE_STRUCT:
			if (FAILURE == zend_hash_find(&ctx->ctx->types,
				   	type.struct_name.val, type.struct_name.len,
					(void**)&tdef)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not find structure %s", type.struct_name.val);
				return NULL;
			}
			ft = &tdef->ffi_t;
			break;
	}

	if (fdef) {
		fdef->intrinsic_type = type.intrinsic_type;
		fdef->type = tdef;
		fdef->ptr_levels = type.ptr_levels;
		if (fdef->ptr_levels) {
			fdef->size = sizeof(void*);
		} else {
			fdef->size = ft->size;
		}
	}
	
	if (type.ptr_levels) {
		return &ffi_type_pointer;
	}
	
	return ft;
}

php_ffi_library *resolve_lib(struct php_ffi_def_context *ctx, php_ffi_ident libname)
{
	php_ffi_library *lib;

	if (FAILURE == zend_hash_find(&ctx->ctx->libraries, libname.val, libname.len, (void**)&lib)) {
		php_ffi_library the_lib = {0};

		the_lib.libname = estrndup(libname.val, libname.len);
		zend_hash_update(&ctx->ctx->libraries, libname.val, libname.len, &the_lib, sizeof(the_lib), (void**)&lib);
	}
	return lib;
}

php_ffi_type_def *register_type(struct php_ffi_def_context *ctx, php_ffi_ident struct_name)
{
	int i;
	php_ffi_type_def tdef = {0}, *rettype;
	CTX_TSRMLS_FETCH();
	long offset = 0;

	tdef.nfields = ctx->n_args;
	tdef.ffi_t.elements = emalloc((tdef.nfields + 1) * sizeof(ffi_type *));
	tdef.ffi_t.elements[tdef.nfields] = NULL; /* ffi wants a NULL terminated array */
	tdef.struct_name = estrndup(struct_name.val, struct_name.len);
	tdef.field_names = emalloc(tdef.nfields * sizeof(char*));
	tdef.field_types = emalloc(tdef.nfields * sizeof(struct php_ffi_field_def));
	
	for (i = 0; i < tdef.nfields; i++) {
		tdef.field_types[i].offset = offset;
		tdef.ffi_t.elements[i] = resolve_type(ctx, ctx->args[i].type, &tdef.field_types[i] TSRMLS_CC);
		tdef.field_names[i] = estrndup(ctx->args[i].name.val, ctx->args[i].name.len);
		offset += ALIGN(tdef.field_types[i].size, SIZEOF_ARG);
	}
	tdef.total_size = offset;
			
	zend_hash_update(&ctx->ctx->types, struct_name.val, struct_name.len, &tdef, sizeof(tdef), (void**)&rettype);

	return rettype;	
}

php_ffi_function *register_func(struct php_ffi_def_context *ctx, php_ffi_type_ref return_type, php_ffi_ident func_name)
{
	php_ffi_function func = {0}, *retfunc;
	int i;
	char funcname[256];
	CTX_TSRMLS_FETCH();

	memcpy(funcname, func_name.val, func_name.len);
	funcname[func_name.len] = '\0';

	func.ret_type = resolve_type(ctx, return_type, NULL TSRMLS_CC);
	func.nargs = ctx->n_args;

	func.lib = resolve_lib(ctx, ctx->libname);

	if (ctx->n_args) {
		func.arg_types = safe_emalloc(ctx->n_args, sizeof(ffi_type *), 0);
		func.arg_info = (zend_arg_info*)safe_emalloc(ctx->n_args, sizeof(zend_arg_info), 0);
		memset(func.arg_info, 0, sizeof(zend_arg_info) * ctx->n_args);
		for (i = 0; i < ctx->n_args; i++) {
			func.arg_types[i] = resolve_type(ctx, ctx->args[i].type, NULL TSRMLS_CC);
			func.arg_info[i].allow_null = func.arg_types[i] == &ffi_type_pointer;
		}
	} else {
		func.arg_types = NULL;
	}
	switch (ffi_prep_cif(&func.cif, FFI_DEFAULT_ABI, ctx->n_args, func.ret_type, func.arg_types)) {
		case FFI_OK:
			zend_hash_update(&ctx->ctx->functions, funcname, func_name.len, &func, sizeof(func), (void**)&retfunc);
			return retfunc;
		default:
			if (func.arg_types) {
				efree(func.arg_types);
			}
	}
	
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not bind call interface for function \"%s\" in library %s", funcname, func.lib->libname);

	return NULL;
}




}

top_level ::= top_list.
top_list ::= top_item top_list.
top_list ::= top_item.

top_item ::= func_proto_with_attributes.
top_item ::= type_def.

arg_type(R) ::= arg_type(T) ASTERISK.	{ R = T; R.type.ptr_levels++; }
arg_type(R) ::= INTRINSIC(N).			{ R = N; R.type.ptr_levels = 0;	}
arg_type(R) ::= STRUCT IDENT(I).		{
   	R.type.intrinsic_type = FFI_TYPE_STRUCT;
	R.type.struct_name = I.ident;
   	R.type.ptr_levels = 0;
}

arg_type(R) ::= IDENT(TNAME).			{ /* TODO */ }

func_attribute_list ::= func_attribute COMMA func_attribute_list.
func_attribute_list ::= func_attribute.

func_attribute ::= IDENT(ATTNAME) EQUALS STRING(VALUE). {
	if (IDENT_EQUALS("lib", ATTNAME.ident)) {
		ctx->libname = VALUE.ident;
	} else {
		CTX_TSRMLS_FETCH();
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported attribute %s", get_ident_string(ATTNAME.ident));
	}
}

optional_func_attribute_list ::= LSQUARE func_attribute_list RSQUARE.

func_proto_with_attributes ::= optional_func_attribute_list func_proto(FP).

func_proto(FP) ::= arg_type(RET) IDENT(NAME) LPAREN arg_list RPAREN SEMI. {
	FP.func = register_func(ctx, RET.type, NAME.ident);
	ctx->n_args = 0;
}


type_def(TD) ::= TYPEDEF arg_type(T) IDENT(I) SEMI.				{
	CTX_TSRMLS_FETCH();
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "aliasing typedefs are not yet implemented");
}

type_def(TD) ::= STRUCT IDENT(I) LBRACE field_list RBRACE SEMI.	{
	register_type(ctx, I.ident);	
	ctx->n_args = 0;
}

field_list ::= field_def field_list.
field_list ::= field_def.

field_def ::= arg_type(T) IDENT(I) SEMI. {
	add_arg(ctx, T.type, I.ident);
}

arg_list ::= argument COMMA arg_list.
arg_list ::= argument.
arg_list ::= .

argument ::= arg_type(T) IDENT(I).	{
	add_arg(ctx, T.type, I.ident);
}
