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


#include "ffi.h"

typedef struct {
	DL_HANDLE handle;		/* handle to the library */
	char *libname;			/* name of the library (for error messages) */
} php_ffi_library;

typedef struct {
	void *func_addr;
	ffi_cif cif;
	int nargs;
	ffi_type **arg_types;
	ffi_type *ret_type;
	zend_arg_info *arg_info;
	php_ffi_library *lib;
} php_ffi_function;

typedef struct {
	HashTable libraries;	/* libraries we loaded */
	HashTable functions; 	/* case sensitive function names -> php_ffi_function */
	HashTable types;		/* structure definitions */
	zend_class_entry *ce;
} php_ffi_context;

typedef struct {
	char *val;
	int len;
} php_ffi_ident;

typedef	struct {
	FFI_TYPE intrinsic_type;
	int ptr_levels;
	php_ffi_ident struct_name;
} php_ffi_type_ref;

typedef struct {
	int nfields;
	char *struct_name;
	ffi_type ffi_t;
	char **field_names;
	struct php_ffi_field_def *field_types;
	long total_size;
} php_ffi_type_def;

struct php_ffi_field_def {
	long offset;
	long size;
	php_ffi_type_def *type;
	FFI_TYPE intrinsic_type;
	int ptr_levels;
};

typedef struct {
	unsigned own_memory:1;	/* 1 if we need to efree the memory */

	/* the chunk of memory */
	char *mem;
	long memlen;

	php_ffi_type_def *tdef;
	
	zend_class_entry *ce;
} php_ffi_struct;



typedef union {
	php_ffi_type_ref type;
	php_ffi_ident ident;
	php_ffi_function *func;
} php_ffi_tokentype;

void *php_ffi_parserAlloc(void *(*mallocProc)(size_t));
void php_ffi_parserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
);

typedef struct {
	php_ffi_type_ref type;
	php_ffi_ident name;
} php_ffi_arg_def;

struct php_ffi_def_context {
	php_ffi_context *ctx;

	int errors, failed;

	int arg_size, n_args;

	php_ffi_arg_def *args;
	php_ffi_type_ref rettype;
	php_ffi_ident funcname;
	php_ffi_ident libname;
#ifdef ZTS
	void **tsrm_ls;
#endif
};

void php_ffi_parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  php_ffi_tokentype yyminor,       /* The value for the token */
  struct php_ffi_def_context *lib             /* Optional %extra_argument parameter */
);
const char *php_ffi_parserTokenName(int tokenType);

#include "ffi_parser.h"

const char *php_ffi_get_token_string(int major, php_ffi_tokentype t);

#define CTX_FETCH(x)	(php_ffi_context*)zend_object_store_get_object(x TSRMLS_CC)
#define STRUCT_FETCH(x)	(php_ffi_struct*)zend_object_store_get_object(x TSRMLS_CC)
#define PHP_FFI_THROW(msg)	zend_throw_exception(zend_exception_get_default(), msg, 0 TSRMLS_CC)

int php_ffi_zval_to_native(void **mem, int *need_free, zval *val, ffi_type *tdef TSRMLS_DC);
int php_ffi_native_to_zval(void *mem, ffi_type *tdef, zval *val TSRMLS_DC);
