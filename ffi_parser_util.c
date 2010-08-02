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
#include "php_ffi.h"
#include "php_ffi_internal.h"

void php_ffi_parser_add_arg(struct php_ffi_def_context *ctx, php_ffi_type_ref type, php_ffi_ident ident)
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

ffi_type *php_ffi_parser_resolve_type(struct php_ffi_def_context *ctx, php_ffi_type_ref type,
	struct php_ffi_field_def *fdef, struct php_ffi_typed_arg *arg TSRMLS_DC)
{
	ffi_type *ft = NULL;
	php_ffi_type_def *tdef = NULL;

	switch (type.intrinsic_type) {
		case FFI_TYPE_VOID:			ft = &ffi_type_void; break;
		case FFI_TYPE_INT:			ft = &ffi_type_sint; break;
		case FFI_TYPE_FLOAT:		ft = &ffi_type_float; break;
		case FFI_TYPE_DOUBLE:		ft = &ffi_type_double; break;
#if SIZEOF_LONG_DOUBLE != SIZEOF_DOUBLE
		case FFI_TYPE_LONGDOUBLE:	ft = &ffi_type_longdouble; break;
#endif
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
		fdef->type.ptr_levels = type.ptr_levels;
		fdef->type.tdef = tdef;
		fdef->type.type = ft;

		if (fdef->type.ptr_levels) {
			fdef->size = sizeof(void*);
		} else {
			fdef->size = ft->size;
		}
	}

	if (arg) {
		arg->tdef = tdef;
		arg->ptr_levels = type.ptr_levels;
		arg->type = ft;
	}
	
	if (type.ptr_levels) {
		return &ffi_type_pointer;
	}
	
	return ft;
}

php_ffi_library *php_ffi_parser_resolve_lib(struct php_ffi_def_context *ctx, php_ffi_ident libname)
{
	php_ffi_library *lib;

	if (FAILURE == zend_hash_find(&ctx->ctx->libraries, libname.val, libname.len, (void**)&lib)) {
		php_ffi_library the_lib = {0};

		the_lib.libname = estrndup(libname.val, libname.len);
		zend_hash_update(&ctx->ctx->libraries, libname.val, libname.len, &the_lib, sizeof(the_lib), (void**)&lib);
	}
	return lib;
}

php_ffi_type_def *php_ffi_parser_register_type(struct php_ffi_def_context *ctx, php_ffi_ident struct_name)
{
	int i;
	php_ffi_type_def tdef = {0}, *rettype;
	long offset = 0;
	CTX_TSRMLS_FETCH();

	tdef.nfields = ctx->n_args;
	tdef.ffi_t.elements = safe_emalloc((tdef.nfields + 1), sizeof(ffi_type *), 0);
	tdef.ffi_t.elements[tdef.nfields] = NULL; /* ffi wants a NULL terminated array */
	tdef.struct_name = estrndup(struct_name.val, struct_name.len);
	tdef.field_names = safe_emalloc(tdef.nfields, sizeof(char*), 0);
	tdef.field_types = safe_emalloc(tdef.nfields, sizeof(struct php_ffi_field_def), 0);
	
	for (i = 0; i < tdef.nfields; i++) {
		tdef.field_types[i].offset = offset;
		tdef.ffi_t.elements[i] = php_ffi_parser_resolve_type(ctx, ctx->args[i].type, &tdef.field_types[i], NULL TSRMLS_CC);
		tdef.field_names[i] = estrndup(ctx->args[i].name.val, ctx->args[i].name.len);
		offset += ALIGN(tdef.field_types[i].size, SIZEOF_ARG);
	}
	tdef.total_size = offset;
			
	zend_hash_update(&ctx->ctx->types, struct_name.val, struct_name.len, &tdef, sizeof(tdef), (void**)&rettype);

	return rettype;	
}

php_ffi_function *php_ffi_parser_register_func(struct php_ffi_def_context *ctx, php_ffi_type_ref return_type, php_ffi_ident func_name)
{
	php_ffi_function func = {0}, *retfunc;
	int i;
	char funcname[256];
	CTX_TSRMLS_FETCH();

	memcpy(funcname, func_name.val, func_name.len);
	funcname[func_name.len] = '\0';

	php_ffi_parser_resolve_type(ctx, return_type, NULL, &func.ret_type TSRMLS_CC);
	func.nargs = ctx->n_args;

	func.lib = php_ffi_parser_resolve_lib(ctx, ctx->libname);

	if (ctx->n_args) {
		func.cif_arg_types = safe_emalloc(ctx->n_args, sizeof(ffi_type *), 0);
		func.arg_types = safe_emalloc(ctx->n_args, sizeof(struct php_ffi_typed_arg), 0);
		func.arg_info = (zend_arg_info*)safe_emalloc(ctx->n_args, sizeof(zend_arg_info), 0);
		memset(func.arg_info, 0, sizeof(zend_arg_info) * ctx->n_args);
		for (i = 0; i < ctx->n_args; i++) {
			func.cif_arg_types[i] = php_ffi_parser_resolve_type(ctx, ctx->args[i].type, NULL, &func.arg_types[i] TSRMLS_CC);
			func.arg_info[i].allow_null = func.cif_arg_types[i] == &ffi_type_pointer;
		}
	} else {
		func.cif_arg_types = NULL;
	}
	switch (ffi_prep_cif(&func.cif, FFI_DEFAULT_ABI, ctx->n_args,
			   func.ret_type.ptr_levels ? &ffi_type_pointer : func.ret_type.type, func.cif_arg_types)) {
		case FFI_OK:
			zend_hash_update(&ctx->ctx->functions, funcname, func_name.len, &func, sizeof(func), (void**)&retfunc);
			return retfunc;
		default:
			if (func.cif_arg_types) {
				efree(func.cif_arg_types);
			}
			if (func.arg_types) {
				efree(func.arg_types);
			}
	}
	
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not bind call interface for function \"%s\" in library %s", funcname, func.lib->libname);

	return NULL;
}




