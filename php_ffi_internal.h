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


#include "ffi.h"

typedef struct {
	DL_HANDLE handle;		/* handle to the library */
	char *libname;			/* name of the library (for error messages) */
} php_ffi_library;

struct php_ffi_typed_arg {
	struct _php_ffi_type_def *tdef;
	int ptr_levels;
	ffi_type *type;
};

typedef struct {
	void *func_addr;
	ffi_cif cif;
	int nargs;
	ffi_type **cif_arg_types;
	struct php_ffi_typed_arg *arg_types;
	struct php_ffi_typed_arg ret_type;
	zend_arg_info *arg_info;
	php_ffi_library *lib;
} php_ffi_function;

typedef struct _php_ffi_type_def {
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
	struct php_ffi_typed_arg type;
};

/* state for ffi class */
typedef struct {
	HashTable libraries;	/* libraries we loaded */
	HashTable functions; 	/* case sensitive function names -> php_ffi_function */
	HashTable types;		/* structure definitions */
	zend_class_entry *ce;	/* needed for inheritance to work */
} php_ffi_context;

/* state for ffi_struct class */
typedef struct {
	unsigned own_memory:1;	/* 1 if we need to efree the memory */

	/* the chunk of memory */
	char *mem;
	long memlen;

	php_ffi_type_def *tdef;
} php_ffi_struct;


/* these structures are only guaranteed to be alive during parsing */

typedef struct {
	char *val;
	int len;
} php_ffi_ident;

typedef	struct {
	FFI_TYPE intrinsic_type;
	int ptr_levels;
	php_ffi_ident struct_name;
} php_ffi_type_ref;

typedef union {
	php_ffi_type_ref type;
	php_ffi_ident ident;
	php_ffi_function *func;
} php_ffi_tokentype;

const char *php_ffi_parserTokenName(int tokenType);
const char *php_ffi_get_token_string(int major, php_ffi_tokentype t);

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

#include "ffi_parser.h"

#ifdef ZTS
# define CTX_TSRMLS_FETCH()	void *tsrm_ls = ctx->tsrm_ls
#else
# define CTX_TSRMLS_FETCH()
#endif

#define IDENT_EQUALS(str, v)	(v.len == sizeof(str)-1 && memcmp(str, v.val, v.len) == 0)

/* end of parser-only structures */

#define CTX_FETCH(x)	(php_ffi_context*)zend_object_store_get_object(x TSRMLS_CC)
#define STRUCT_FETCH(x)	(php_ffi_struct*)zend_object_store_get_object(x TSRMLS_CC)
#define PHP_FFI_THROW(msg)	zend_throw_exception(zend_exception_get_default(TSRMLS_C), msg, 0 TSRMLS_CC)

int php_ffi_zval_to_native(void **mem, int *need_free, zval *val, struct php_ffi_typed_arg *argtype TSRMLS_DC);
int php_ffi_native_to_zval(void *mem, struct php_ffi_typed_arg *argtype, zval *val TSRMLS_DC);

void php_ffi_parser_add_arg(struct php_ffi_def_context *ctx, php_ffi_type_ref type, php_ffi_ident ident);
ffi_type *php_ffi_parser_resolve_type(struct php_ffi_def_context *ctx, php_ffi_type_ref type,
	struct php_ffi_field_def *fdef, struct php_ffi_typed_arg *arg TSRMLS_DC);

php_ffi_library *php_ffi_parser_resolve_lib(struct php_ffi_def_context *ctx, php_ffi_ident libname);
php_ffi_type_def *php_ffi_parser_register_type(struct php_ffi_def_context *ctx, php_ffi_ident struct_name);
php_ffi_function *php_ffi_parser_register_func(struct php_ffi_def_context *ctx, php_ffi_type_ref return_type, php_ffi_ident func_name);

void php_ffi_struct_dtor(void *object, zend_object_handle handle TSRMLS_DC);
void php_ffi_struct_object_clone(void *object, void **clone_ptr TSRMLS_DC);
extern zend_object_handlers php_ffi_struct_object_handlers;

SINT64 php_ffi_strto_int64(const char *nptr, char **endptr, int base, int is_unsigned);
char *php_ffi_int64_tostr(SINT64 val, char *dst, int radix);

#define UPHP_FFI_SINT64_MAX	(~(UINT64)0)
#define PHP_FFI_SINT64_MIN	((SINT64)0x8000000000000000)
#define PHP_FFI_SINT64_MAX	((SINT64)0x7FFFFFFFFFFFFFFF)

