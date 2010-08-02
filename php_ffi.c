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

/* If you declare any globals in php_ffi.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(ffi)
*/

/* True global resources - no need for thread safety here */
static int le_ffi;

zend_class_entry *php_ffi_struct_class_entry = NULL,
	*php_ffi_context_class_entry = NULL;

/* {{{ ffi_functions[]
 *
 * Every user visible function must have an entry in ffi_functions[].
 */
zend_function_entry ffi_functions[] = {
	{NULL, NULL, NULL}	/* Must be the last line in ffi_functions[] */
};
/* }}} */

/* {{{ ffi_module_entry
 */
zend_module_entry ffi_module_entry = {
	STANDARD_MODULE_HEADER,
	"ffi",
	ffi_functions,
	PHP_MINIT(ffi),
	PHP_MSHUTDOWN(ffi),
	PHP_RINIT(ffi),
	PHP_RSHUTDOWN(ffi),
	PHP_MINFO(ffi),
	PHP_FFI_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FFI
ZEND_GET_MODULE(ffi)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("ffi.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_ffi_globals, ffi_globals)
    STD_PHP_INI_ENTRY("ffi.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_ffi_globals, ffi_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_ffi_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_ffi_init_globals(zend_ffi_globals *ffi_globals)
{
	ffi_globals->global_value = 0;
	ffi_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ffi)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "FFI", php_ffi_context_funcs);
	ce.create_object = php_ffi_context_object_new;
	php_ffi_context_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "FFIStruct", NULL);
	ce.create_object = php_ffi_struct_object_new;
	php_ffi_struct_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
		
	/* If you have INI entries, uncomment these lines 
	ZEND_INIT_MODULE_GLOBALS(ffi, php_ffi_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ffi)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ffi)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ffi)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ffi)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ffi support", "enabled");
	php_info_print_table_row(2, "extension version", PHP_FFI_VERSION);
/*	php_info_print_table_row(2, "ffi library version", PHP_LIBFFI_VERSION); */
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
