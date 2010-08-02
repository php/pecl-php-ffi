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

#ifndef PHP_FFI_H
#define PHP_FFI_H

extern zend_module_entry ffi_module_entry;
#define phpext_ffi_ptr &ffi_module_entry

#define PHP_FFI_VERSION "0.4.0-dev"

#ifdef PHP_WIN32
#define PHP_FFI_API __declspec(dllexport)
#else
#define PHP_FFI_API
#endif

#ifndef  Z_SET_REFCOUNT_P
# if PHP_MAJOR_VERSION < 6 && (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
#  define Z_SET_REFCOUNT_P(pz, rc)  pz->refcount = rc 
#  define Z_UNSET_ISREF_P(pz) pz->is_ref = 0 
# endif
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(ffi);
PHP_MSHUTDOWN_FUNCTION(ffi);
PHP_RINIT_FUNCTION(ffi);
PHP_RSHUTDOWN_FUNCTION(ffi);
PHP_MINFO_FUNCTION(ffi);

zend_object_value php_ffi_context_object_new(zend_class_entry *ce TSRMLS_DC);
zend_object_value php_ffi_struct_object_new(zend_class_entry *ce TSRMLS_DC);
extern zend_class_entry *php_ffi_struct_class_entry, *php_ffi_context_class_entry;
extern zend_function_entry php_ffi_struct_funcs[];
extern zend_function_entry php_ffi_context_funcs[];

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(ffi)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(ffi)
*/

/* In every utility function you add that needs to use variables 
   in php_ffi_globals, call TSRM_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as FFI_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define FFI_G(v) TSRMG(ffi_globals_id, zend_ffi_globals *, v)
#else
#define FFI_G(v) (ffi_globals.v)
#endif

#endif	/* PHP_FFI_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
