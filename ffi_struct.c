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
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ffi.h"
#include "Zend/zend_default_classes.h"

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

static zval *php_ffi_struct_property_read(zval *object, zval *member, zend_bool silent TSRMLS_DC)
{
	zval *return_value;
	php_ffi_struct *obj;
	int i;

	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);

	obj = STRUCT_FETCH(object);

	/* look for the property */
	for (i = 0; i < obj->tdef->nfields; i++) {
		if (strcmp(Z_STRVAL_P(member), obj->tdef->field_names[i]) == 0) {
			/* TODO: read and convert the memory to a zval */
			return return_value;
		}
	}

	PHP_FFI_THROW("no such property");

	return return_value;
}

static void php_ffi_struct_property_write(zval *object, zval *member, zval *value TSRMLS_DC)
{
#if 0
	zval *return_value;
	php_ffi_struct *obj;
	int i;

	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);

	obj = STRUCT_FETCH(object);

	/* look for the property */
	for (i = 0; i < obj->tdef->nfields; i++) {
		if (strcmp(Z_STRVAL_P(member), obj->tdef->field_names[i]) == 0) {
			/* TODO: read and convert the memory to a zval */
			return return_value;
		}
	}

	PHP_FFI_THROW("no such property");

	return return_value;
#endif
}

static zval *php_ffi_struct_read_dimension(zval *object, zval *offset TSRMLS_DC)
{
	zval *return_value;

	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);

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
	zval *return_value;
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
	zend_internal_function *f;
	php_ffi_struct *obj;
	php_ffi_function *func;
	
	obj = STRUCT_FETCH(object);

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
	zend_internal_function *f;

	obj = STRUCT_FETCH(object);

	f = emalloc(sizeof(zend_internal_function));
	f->type = ZEND_INTERNAL_FUNCTION;
	f->function_name = obj->ce->name;
	f->scope = obj->ce;
	f->arg_info = NULL;
	f->num_args = 0;
	f->fn_flags = 0;
	f->handler = ZEND_FN(php_ffi_struct_create_instance);
	return (union _zend_function*)f;
}

static zend_class_entry *php_ffi_struct_class_entry_get(zval *object TSRMLS_DC)
{
	php_ffi_struct *obj;
	obj = STRUCT_FETCH(object);

	return obj->ce;
}

static int php_ffi_struct_class_name_get(zval *object, char **class_name, zend_uint *class_name_len, int parent TSRMLS_DC)
{
	php_ffi_struct *obj;
	obj = STRUCT_FETCH(object);

	*class_name = estrndup(obj->ce->name, obj->ce->name_length);
	*class_name_len = obj->ce->name_length;

	return 0;
}

static int php_ffi_struct_objects_compare(zval *object1, zval *object2 TSRMLS_DC)
{
	return -1;
}

static int php_ffi_struct_object_cast(zval *readobj, zval *writeobj, int type, int should_free TSRMLS_DC)
{
	if (should_free) {
		zval_dtor(writeobj);
	}

	ZVAL_NULL(writeobj);

	return FAILURE;
}

static zend_object_handlers php_ffi_struct_object_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,
	php_ffi_struct_property_read,
	php_ffi_struct_property_write,
	php_ffi_struct_read_dimension,
	php_ffi_struct_write_dimension,
	NULL,
	php_ffi_struct_object_get,
	php_ffi_struct_object_set,
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
	php_ffi_struct_object_cast
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
	obj->ce = ce;
	
	retval.handle = zend_objects_store_put(obj, php_ffi_struct_dtor, php_ffi_struct_object_clone TSRMLS_CC);
	retval.handlers = &php_ffi_struct_object_handlers;

	return retval;
}

/* Given a zval and a type spec, copy or reference the data from
 * the zval and set *mem to that thing */
int php_ffi_zval_to_native(void **mem, int *need_free, zval *val, ffi_type *tdef TSRMLS_DC)
{
	*need_free = 0;

	switch (tdef->type) {
		case FFI_TYPE_VOID:		*mem = NULL; return 1;
		case FFI_TYPE_POINTER:
			/* TODO: should verify that we are really looking for a string */
			convert_to_string(val);
			*mem = &Z_STRVAL_P(val);
			return 1;
		case FFI_TYPE_INT:
			convert_to_long(val);
			*mem = &Z_LVAL_P(val);
			return 1;
	}

	/* other stuff that we can't safely detect using the switch statement */
	if (tdef == &ffi_type_slong || tdef == &ffi_type_sint || tdef == &ffi_type_uint32 || tdef == &ffi_type_sint32) {
		convert_to_long(val);
		*mem = &Z_LVAL_P(val);
		return 1;
	} else if (tdef == &ffi_type_uint8) {
		convert_to_long(val);
		*mem = emalloc(sizeof(char));
		*(unsigned char*)mem = (unsigned char)Z_LVAL_P(val);
		*need_free = 1;
		return 1;
	} else if (tdef == &ffi_type_sint8) {
		convert_to_long(val);
		*mem = emalloc(sizeof(char));
		*(char*)mem = (char)Z_LVAL_P(val);
		*need_free = 1;
		return 1;
	} else if (tdef == &ffi_type_uint16) {
		convert_to_long(val);
		*mem = emalloc(sizeof(short));
		*(unsigned short*)mem = (unsigned short)Z_LVAL_P(val);
		*need_free = 1;
		return 1;
	} else if (tdef == &ffi_type_sint16) {
		convert_to_long(val);
		*mem = emalloc(sizeof(short));
		*(short*)mem = (short)Z_LVAL_P(val);
		*need_free = 1;
		return 1;
	} else if (tdef == &ffi_type_double) {
		convert_to_double(val);
		*mem = &Z_DVAL_P(val);
		return 1;
	}

	return 0;
}

int php_ffi_native_to_zval(void *mem, ffi_type *tdef, zval *val TSRMLS_DC)
{
	if (tdef == &ffi_type_slong || tdef == &ffi_type_sint || tdef == &ffi_type_uint32 || tdef == &ffi_type_sint32) {
		ZVAL_LONG(val, *(long*)mem);
		return 1;
	} else if (tdef == &ffi_type_uint8) {
		ZVAL_LONG(val, *(unsigned char*)mem);
		return 1;
	} else if (tdef == &ffi_type_sint8) {
		ZVAL_LONG(val, *(char*)mem);
		return 1;
	} else if (tdef == &ffi_type_uint16) {
		ZVAL_LONG(val, *(unsigned short*)mem);
		return 1;
	} else if (tdef == &ffi_type_sint16) {
		ZVAL_LONG(val, *(short*)mem);
		return 1;
	} else if (tdef == &ffi_type_double) {
		ZVAL_DOUBLE(val, *(double*)mem);
		return 1;
	}
	return 0;
}

