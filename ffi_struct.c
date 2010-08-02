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

#include "php_ffi_internal.h"

static PHP_FUNCTION(php_ffi_struct_create_instance)
{
	zval *ctxobj;
	zval *object = getThis();
	php_ffi_struct *obj;
	php_ffi_context *ctx;
	char *structname, *initdata = NULL;
	long structnamelen, initdatalen = 0;

	obj = STRUCT_FETCH(object);

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "os|s",
			&ctxobj, &structname, &structnamelen,
			&initdata, &initdatalen)) {
		ZVAL_NULL(object);
		return;
	}

	ctx = CTX_FETCH(ctxobj);

	if (FAILURE == zend_hash_find(&ctx->types, structname, structnamelen, (void**)&obj->tdef)) {
		PHP_FFI_THROW("no such structure type");
		ZVAL_NULL(object);
		return;
	}
	obj->memlen = obj->tdef->total_size;
	obj->mem = ecalloc(1, obj->memlen);
	obj->own_memory = 1;

	if (initdata) {
		if (initdatalen != obj->memlen) {
			PHP_FFI_THROW("initializer has incorrect length");
			ZVAL_NULL(object);
			return;
		}
		memcpy(obj->mem, initdata, obj->memlen);
	}
}

static zval *php_ffi_struct_property_read(zval *object, zval *member, int type TSRMLS_DC)
{
	zval *return_value;
	php_ffi_struct *obj;
	int i;

	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);
	Z_SET_REFCOUNT_P(return_value, 0);
	Z_UNSET_ISREF_P(return_value);

	obj = STRUCT_FETCH(object);

	if (strcmp(Z_STRVAL_P(member), "__asBinary") == 0) {
		ZVAL_STRINGL(return_value, obj->mem, obj->memlen, 1);
		return return_value;
	} else if (strcmp(Z_STRVAL_P(member), "__sizeof") == 0) {
		ZVAL_LONG(return_value, obj->memlen);
		return return_value;
	}
	
	/* look for the property */
	for (i = 0; i < obj->tdef->nfields; i++) {
		if (strcmp(Z_STRVAL_P(member), obj->tdef->field_names[i]) == 0) {
			/* figure out address of this member */
			char *addr = obj->mem + obj->tdef->field_types[i].offset;
			
			/* TODO: handle the case where ptr_levels is non zero! */
			if (!php_ffi_native_to_zval(addr, &obj->tdef->field_types[i].type, return_value TSRMLS_CC)) {
				PHP_FFI_THROW("could not map property!");
			}
			return return_value;
		}
	}

	PHP_FFI_THROW("no such property");

	return return_value;
}

static void php_ffi_struct_property_write(zval *object, zval *member, zval *value TSRMLS_DC)
{
	php_ffi_struct *obj;
	int i;

	obj = STRUCT_FETCH(object);

	if (strcmp(Z_STRVAL_P(member), "__asBinary") == 0) {
		if (Z_TYPE_P(value) != IS_STRING)  {
			PHP_FFI_THROW("__as_binary value in assignment *MUST* be a string");
			return;
		}
		if (Z_STRLEN_P(value) != obj->memlen) {
			PHP_FFI_THROW("length of binary string *MUST* match size of structure");
			return;
		}

		memcpy(obj->mem, Z_STRVAL_P(value), obj->memlen);
		return;
	}
	
	/* look for the property */
	for (i = 0; i < obj->tdef->nfields; i++) {
		if (strcmp(Z_STRVAL_P(member), obj->tdef->field_names[i]) == 0) {
			/* TODO: read and convert the memory to a zval */
			int need_free = 0;
			char *addr = obj->mem + obj->tdef->field_types[i].offset;
			if (!php_ffi_zval_to_native((void**)&addr, &need_free, value, &obj->tdef->field_types[i].type TSRMLS_CC)) {
				PHP_FFI_THROW("could not map property!");
			}

			return;
		}
	}

	PHP_FFI_THROW("no such property");
}

static zval *php_ffi_struct_read_dimension(zval *object, zval *offset, int type TSRMLS_DC)
{
	zval *return_value;

	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);
	Z_SET_REFCOUNT_P(return_value, 0);
	Z_UNSET_ISREF_P(return_value);

	PHP_FFI_THROW("no dimension support yet");
	return return_value;
}

static void php_ffi_struct_write_dimension(zval *object, zval *offset, zval *value TSRMLS_DC)
{
	PHP_FFI_THROW("no dimension support yet");
}

static void php_ffi_struct_object_set(zval **property, zval *value TSRMLS_DC)
{
	/* Not yet implemented in the engine */
}

static zval *php_ffi_struct_object_get(zval *property TSRMLS_DC)
{
	/* Not yet implemented in the engine */
	return NULL;
}

static int php_ffi_struct_property_exists(zval *object, zval *member, int check_empty TSRMLS_DC)
{
	php_ffi_struct *obj;
	int i;

	obj = STRUCT_FETCH(object);

	/* look for the property */
	for (i = 0; i < obj->tdef->nfields; i++) {
		if (strcmp(Z_STRVAL_P(member), obj->tdef->field_names[i]) == 0) {
			return 1;
		}
	}

	return 0;
}

static void php_ffi_struct_property_delete(zval *object, zval *member TSRMLS_DC)
{
	PHP_FFI_THROW("Cannot delete properties from an ffi_struct");
}

static int php_ffi_struct_dimension_exists(zval *object, zval *member, int check_empty TSRMLS_DC)
{
	return 0;
}

static void php_ffi_struct_dimension_delete(zval *object, zval *offset TSRMLS_DC)
{
	PHP_FFI_THROW("Cannot delete properties from an ffi_struct");
}

static HashTable *php_ffi_struct_properties_get(zval *object TSRMLS_DC)
{
	/* TODO */
	return NULL;
}

static union _zend_function *php_ffi_struct_method_get(zval *object, char *name, int len TSRMLS_DC)
{
#if 0
	php_ffi_struct *obj = STRUCT_FETCH(object);
#endif
	/* TODO: worry about pointers to functions? */
	
	return NULL;
}

static int php_ffi_struct_call_method(char *method, INTERNAL_FUNCTION_PARAMETERS)
{
	php_ffi_struct *obj;

	obj = STRUCT_FETCH(getThis());

	return FAILURE;
}

static union _zend_function *php_ffi_struct_constructor_get(zval *object TSRMLS_DC)
{
	php_ffi_struct *obj;
	static zend_internal_function f;

	obj = STRUCT_FETCH(object);

	f.type = ZEND_INTERNAL_FUNCTION;
	f.function_name = php_ffi_struct_class_entry->name;
	f.scope = php_ffi_struct_class_entry;
	f.arg_info = NULL;
	f.num_args = 0;
	f.fn_flags = 0;
	f.handler = ZEND_FN(php_ffi_struct_create_instance);
	return (union _zend_function*)&f;
}

static zend_class_entry *php_ffi_struct_class_entry_get(zval *object TSRMLS_DC)
{
	return php_ffi_struct_class_entry;
}

static int php_ffi_struct_class_name_get(zval *object, char **class_name, zend_uint *class_name_len, int parent TSRMLS_DC)
{
	php_ffi_struct *obj;
	obj = STRUCT_FETCH(object);

	*class_name = estrndup(php_ffi_struct_class_entry->name, php_ffi_struct_class_entry->name_length);
	*class_name_len = php_ffi_struct_class_entry->name_length;

	return 0;
}

static int php_ffi_struct_objects_compare(zval *object1, zval *object2 TSRMLS_DC)
{
	return -1;
}

static int php_ffi_struct_object_cast(zval *readobj, zval *writeobj, int type TSRMLS_DC)
{
	return FAILURE;
}

zend_object_handlers php_ffi_struct_object_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,
	php_ffi_struct_property_read,
	php_ffi_struct_property_write,
	php_ffi_struct_read_dimension,
	php_ffi_struct_write_dimension,
	NULL,
	NULL, /* php_ffi_struct_object_get, */
	NULL, /* php_ffi_struct_object_set, */
	php_ffi_struct_property_exists,
	php_ffi_struct_property_delete,
	php_ffi_struct_dimension_exists,
	php_ffi_struct_dimension_delete,
	php_ffi_struct_properties_get,
	php_ffi_struct_method_get,
	php_ffi_struct_call_method,
	php_ffi_struct_constructor_get,
	php_ffi_struct_class_entry_get,
	php_ffi_struct_class_name_get,
	php_ffi_struct_objects_compare,
	php_ffi_struct_object_cast,
	NULL, /* count */
};

void php_ffi_struct_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
	php_ffi_struct *obj = (php_ffi_struct*)object;

	if (obj->own_memory) {
		efree(obj->mem);
	}

	efree(obj);
}

void php_ffi_struct_object_clone(void *object, void **clone_ptr TSRMLS_DC)
{
	php_ffi_struct *cloneobj, *origobject;

	origobject = (php_ffi_struct*)object;
	cloneobj = (php_ffi_struct*)emalloc(sizeof(*cloneobj));
	
	memcpy(cloneobj, origobject, sizeof(*cloneobj));

	if (origobject->own_memory) {
		cloneobj->mem = emalloc(origobject->memlen);
		memcpy(cloneobj->mem, origobject->mem, origobject->memlen);
	}

	*clone_ptr = cloneobj;
}

zend_object_value php_ffi_struct_object_new(zend_class_entry *ce TSRMLS_DC)
{
	php_ffi_struct *obj;
	zend_object_value retval;

	obj = ecalloc(1, sizeof(*obj));
	
	retval.handle = zend_objects_store_put(obj, php_ffi_struct_dtor, NULL, php_ffi_struct_object_clone TSRMLS_CC);
	retval.handlers = &php_ffi_struct_object_handlers;

	return retval;
}


/* Given a zval and a type spec, copy or reference the data from
 * the zval and set *mem to point to that thing */
int php_ffi_zval_to_native(void **mem, int *need_free, zval *val, struct php_ffi_typed_arg *argtype TSRMLS_DC)
{
	int want_alloc = *need_free;
	*need_free = 0;

	if (argtype->ptr_levels && Z_TYPE_P(val) == IS_NULL) {
		if (want_alloc) {
			*mem = emalloc(sizeof(void*));
			*need_free = 1;
		}
		*(void**)*mem = NULL;
		return 1;
	} else if (argtype->ptr_levels == 1) {
		if (Z_TYPE_P(val) == IS_OBJECT) {
			if (Z_OBJCE_P(val) == php_ffi_struct_class_entry) {
				php_ffi_struct *str = STRUCT_FETCH(val);
				*mem = &str->mem;
				return 1;
			}
			return 0;
		}
		convert_to_string(val);
		*mem = &Z_STRVAL_P(val);
		return 1;
	} else if (argtype->ptr_levels) {
		return 0;
	}

	/* so it's not a pointer */

	if (argtype->type == &ffi_type_slong || argtype->type == &ffi_type_sint ||
			argtype->type == &ffi_type_uint32 || argtype->type == &ffi_type_sint32) {

		if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 1) {
			if (want_alloc) {
				*mem = emalloc(sizeof(long));
				*need_free = 1;
			}
			**(long**)mem = Z_STRVAL_P(val)[0];
		} else {
			convert_to_long(val);
			*(long **)mem = &Z_LVAL_P(val);
		}
		return 1;
	} else if (argtype->type == &ffi_type_uint8) {
		if (want_alloc) {
			*mem = emalloc(sizeof(char));
			*need_free = 1;
		}
		if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 1) {
			**(unsigned char**)mem = Z_STRVAL_P(val)[0];
		} else {
			convert_to_long(val);
			**(unsigned char**)mem = (unsigned char)Z_LVAL_P(val);
		}
		return 1;
	} else if (argtype->type == &ffi_type_sint8) {
		if (want_alloc) {
			*mem = emalloc(sizeof(char));
			*need_free = 1;
		}
		if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 1) {
			**(char**)mem = Z_STRVAL_P(val)[0];
		} else {
			convert_to_long(val);
			**(char**)mem = (char)Z_LVAL_P(val);
		}
		return 1;
	} else if (argtype->type == &ffi_type_uint16) {
		if (want_alloc) {
			*mem = emalloc(sizeof(short));
			*need_free = 1;
		}
		if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 1) {
			**(unsigned short**)mem = Z_STRVAL_P(val)[0];
		} else {
			convert_to_long(val);
			**(unsigned short**)mem = (unsigned short)Z_LVAL_P(val);
		}
		return 1;
	} else if (argtype->type == &ffi_type_sint16) {
		if (want_alloc) {
			*mem = emalloc(sizeof(short));
			*need_free = 1;
		}
		if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 1) {
			**(short**)mem = Z_STRVAL_P(val)[0];
		} else {
			convert_to_long(val);
			**(short**)mem = (short)Z_LVAL_P(val);
		}
		return 1;
	} else if (argtype->type == &ffi_type_double) {
		convert_to_double(val);
		*mem = &Z_DVAL_P(val);
		return 1;
	} else if (argtype->type == &ffi_type_float) {
		convert_to_double(val);
		if (want_alloc) {
			*mem = emalloc(sizeof(float));
			*need_free = 1;
		}
		**(float**)mem = (float)Z_DVAL_P(val);
		return 1;
	} else if (argtype->type == &ffi_type_sint64 || argtype->type == &ffi_type_uint64) {
		if (want_alloc) {
			*mem = emalloc(sizeof(SINT64));
			*need_free = 1;
		}	
		if (Z_TYPE_P(val) == IS_LONG) {
			**(SINT64**)mem = Z_LVAL_P(val);
		} else {
			convert_to_string(val);
			**(SINT64**)mem = php_ffi_strto_int64(Z_STRVAL_P(val), NULL, -1, argtype->type == &ffi_type_uint64);
		}
		return 1;	
	}

	return 0;
}

int php_ffi_native_to_zval(void *mem, struct php_ffi_typed_arg *argtype, zval *val TSRMLS_DC)
{
	php_ffi_struct *str;
	
	if (argtype->ptr_levels == 1) {
		if (argtype->tdef) {
			/* pointer to a structure type */

			str = ecalloc(1, sizeof(*str));
			str->mem = *(char**)mem;
			str->tdef = argtype->tdef;
			str->memlen = str->tdef->total_size;

			Z_TYPE_P(val) = IS_OBJECT;
			Z_OBJ_HANDLE_P(val) = zend_objects_store_put(str,
					php_ffi_struct_dtor, NULL, php_ffi_struct_object_clone TSRMLS_CC);
			Z_OBJ_HT_P(val) = &php_ffi_struct_object_handlers;
			return 1;
		}
		if (argtype->type == &ffi_type_uint8 || argtype->type == &ffi_type_sint8) {
			ZVAL_STRING(val, *(char**)mem, 1);
			return 1;
		}
		return 0;
	}
	
	if (argtype->type == &ffi_type_slong || argtype->type == &ffi_type_sint ||
			argtype->type == &ffi_type_uint32 || argtype->type == &ffi_type_sint32) {
		ZVAL_LONG(val, *(long*)mem);
		return 1;
	} else if (argtype->type == &ffi_type_uint8) {
		ZVAL_LONG(val, *(unsigned char*)mem);
		return 1;
	} else if (argtype->type == &ffi_type_sint8) {
		ZVAL_LONG(val, *(char*)mem);
		return 1;
	} else if (argtype->type == &ffi_type_uint16) {
		ZVAL_LONG(val, *(unsigned short*)mem);
		return 1;
	} else if (argtype->type == &ffi_type_sint16) {
		ZVAL_LONG(val, *(short*)mem);
		return 1;
	} else if (argtype->type == &ffi_type_double) {
		ZVAL_DOUBLE(val, *(double*)mem);
		return 1;
	} else if (argtype->type == &ffi_type_float) {
		ZVAL_DOUBLE(val, *(float*)mem);
		return 1;
	} else if (argtype->type == &ffi_type_sint64 || argtype->type == &ffi_type_uint64) {
		char intbuf[128];
		php_ffi_int64_tostr(*(SINT64*)mem, intbuf, -1);
		ZVAL_STRING(val, intbuf, 1);
		return 1;
	}

	return 0;
}

