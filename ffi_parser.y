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
#if SIZEOF_LONG_DOUBLE != SIZEOF_DOUBLE
				case FFI_TYPE_LONGDOUBLE: c = "long double"; break;
#endif
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
#if ZEND_DEBUG
			sprintf(tokbuf, "token: %s", php_ffi_parserTokenName(major));
#else
			sprintf(tokbuf, "token: ???");
#endif
	}
	return tokbuf;
}
}

top_level ::= top_list.
top_list ::= top_item top_list.
top_list ::= top_item.

top_item ::= func_proto_with_attributes.
top_item ::= func_proto.
top_item ::= type_def.
top_item ::= callback_def.

optional_asterisk_list(OL) ::= .					{ OL.type.ptr_levels = 0; }
optional_asterisk_list(OL) ::= asterisk_list(AL).	{ OL = AL; }

asterisk_list(AL) ::= ASTERISK.						{ AL.type.ptr_levels = 1; }
asterisk_list(AL) ::= asterisk_list(L) ASTERISK.	{ AL = L; AL.type.ptr_levels++; }

arg_type(R) ::= INTRINSIC(N) optional_asterisk_list(OAL).	  { R = N; R.type.ptr_levels = OAL.type.ptr_levels; }
arg_type(R) ::= STRUCT IDENT(I) optional_asterisk_list(OAL).  {
   	R.type.intrinsic_type = FFI_TYPE_STRUCT;
	R.type.struct_name = I.ident;
   	R.type.ptr_levels = OAL.type.ptr_levels;
}

arg_type(R) ::= IDENT(TNAME).			{
	CTX_TSRMLS_FETCH();
	/* TODO: lookup ident (including callback types) */
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unrecognized type name `%s'", get_ident_string(TNAME.ident));	
}

/* define a callback function type; once defined, the CBNAME can be used
 * just like any other type */
callback_def ::= CALLBACK arg_type(T) IDENT(CBNAME) LPAREN arg_list RPAREN SEMI. {
	CTX_TSRMLS_FETCH();
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "callback support not complete");	
}

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

func_proto_with_attributes ::= LSQUARE func_attribute_list RSQUARE func_proto(FP).

func_proto(FP) ::= arg_type(RET) IDENT(NAME) LPAREN arg_list RPAREN SEMI. {
	FP.func = php_ffi_parser_register_func(ctx, RET.type, NAME.ident);
	ctx->n_args = 0;
}

type_def(TD) ::= TYPEDEF arg_type(T) IDENT(I) SEMI.				{
	CTX_TSRMLS_FETCH();
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "aliasing typedefs are not yet implemented");
}

type_def(TD) ::= STRUCT IDENT(I) LBRACE field_list RBRACE SEMI.	{
	php_ffi_parser_register_type(ctx, I.ident);	
	ctx->n_args = 0;
}

field_list ::= field_def field_list.
field_list ::= field_def.

field_def ::= arg_type(T) IDENT(I) SEMI. {
	php_ffi_parser_add_arg(ctx, T.type, I.ident);
}

arg_list ::= argument COMMA arg_list.
arg_list ::= argument.
arg_list ::= .

argument ::= arg_type(T) IDENT(I).	{
	php_ffi_parser_add_arg(ctx, T.type, I.ident);
}
