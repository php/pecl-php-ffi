/* Driver template for the LEMON parser generator.
** Copyright 1991-1995 by D. Richard Hipp.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
** Modified 1997 to make it suitable for use with makeheaders.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
#line 7 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"

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

static char *yyTokenName[];

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
			sprintf(tokbuf, "token: %s", yyTokenName[major]);
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
	long offset = 0;
	CTX_TSRMLS_FETCH();

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





#line 268 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    php_ffi_parserTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is php_ffi_parserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.
**    php_ffi_parserARGDECL       is a declaration of a 3rd argument to the
**                       parser, or null if there is no extra argument.
**    php_ffi_parserKRARGDECL     A version of php_ffi_parserARGDECL for K&R C.
**    php_ffi_parserANSIARGDECL   A version of php_ffi_parserARGDECL for ANSI C.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
/*  */
#define YYCODETYPE unsigned char
#define YYNOCODE 32
#define YYACTIONTYPE unsigned char
#define php_ffi_parserTOKENTYPE  php_ffi_tokentype 
typedef union {
  php_ffi_parserTOKENTYPE yy0;
  int yy63;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define php_ffi_parserARGDECL ,ctx 
#define php_ffi_parserXARGDECL  struct php_ffi_def_context *ctx ;
#define php_ffi_parserANSIARGDECL , struct php_ffi_def_context *ctx 
#define YYNSTATE 48
#define YYNRULE 24
#define YYERRORSYMBOL 19
#define YYERRSYMDT yy63
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)
/* Next is the action table.  Each entry in this table contains
**
**  +  An integer which is the number representing the look-ahead
**     token
**
**  +  An integer indicating what action to take.  Number (N) between
**     0 and YYNSTATE-1 mean shift the look-ahead and go to state N.
**     Numbers between YYNSTATE and YYNSTATE+YYNRULE-1 mean reduce by
**     rule N-YYNSTATE.  Number YYNSTATE+YYNRULE means that a syntax
**     error has occurred.  Number YYNSTATE+YYNRULE+1 means the parser
**     accepts its input.
**
**  +  A pointer to the next entry with the same hash value.
**
** The action table is really a series of hash tables.  Each hash
** table contains a number of entries which is a power of two.  The
** "state" table (which follows) contains information about the starting
** point and size of each hash table.
*/
struct yyActionEntry {
  YYCODETYPE   lookahead;   /* The value of the look-ahead token */
  YYACTIONTYPE action;      /* Action to take for this look-ahead */
  struct yyActionEntry *next; /* Next look-ahead with the same hash, or NULL */
};
static struct yyActionEntry yyActionTable[] = {
/* State 0 */
  {  14,  37, 0                    }, /*                 STRUCT shift  37 */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {   8,   6, 0                    }, /*                LSQUARE shift  6 */
  {  25,   4, 0                    }, /* func_proto_with_attributes shift  4 */
  {  26,  15, 0                    }, /* optional_func_attribute_list shift  15 */
  {  27,   2, 0                    }, /*               top_item shift  2 */
  {  28,  73, 0                    }, /*              top_level accept */
  {  29,   1, 0                    }, /*               top_list shift  1 */
  {  30,   5, &yyActionTable[   0] }, /*               type_def shift  5 */
  {  15,  33, 0                    }, /*                TYPEDEF shift  33 */
/* State 1 */
  {   0,  48, 0                    }, /*                      $ reduce 0 */
/* State 2 */
  {   0,  50, 0                    }, /*                      $ reduce 2 */
  {  14,  37, 0                    }, /*                 STRUCT shift  37 */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {   8,   6, 0                    }, /*                LSQUARE shift  6 */
  {  25,   4, 0                    }, /* func_proto_with_attributes shift  4 */
  {  26,  15, 0                    }, /* optional_func_attribute_list shift  15 */
  {  27,   2, 0                    }, /*               top_item shift  2 */
  {YYNOCODE,0,0}, /* Unused */
  {  29,   3, 0                    }, /*               top_list shift  3 */
  {  30,   5, &yyActionTable[  18] }, /*               type_def shift  5 */
  {  15,  33, 0                    }, /*                TYPEDEF shift  33 */
/* State 3 */
  {   0,  49, 0                    }, /*                      $ reduce 1 */
/* State 4 */
  {YYNOCODE,0,0}, /* Unused */
/* State 5 */
  {YYNOCODE,0,0}, /* Unused */
/* State 6 */
  {   4,  10, 0                    }, /*                  IDENT shift  10 */
  {YYNOCODE,0,0}, /* Unused */
  {  22,   7, 0                    }, /*         func_attribute shift  7 */
  {  23,  13, 0                    }, /*    func_attribute_list shift  13 */
/* State 7 */
  {   2,   8, 0                    }, /*                  COMMA shift  8 */
  {  11,  58, 0                    }, /*                RSQUARE reduce 10 */
/* State 8 */
  {   4,  10, 0                    }, /*                  IDENT shift  10 */
  {YYNOCODE,0,0}, /* Unused */
  {  22,   7, 0                    }, /*         func_attribute shift  7 */
  {  23,   9, 0                    }, /*    func_attribute_list shift  9 */
/* State 9 */
  {  11,  57, 0                    }, /*                RSQUARE reduce 9 */
/* State 10 */
  {   3,  11, 0                    }, /*                 EQUALS shift  11 */
/* State 11 */
  {  13,  12, 0                    }, /*                 STRING shift  12 */
/* State 12 */
  {YYNOCODE,0,0}, /* Unused */
/* State 13 */
  {  11,  14, 0                    }, /*                RSQUARE shift  14 */
/* State 14 */
  {YYNOCODE,0,0}, /* Unused */
/* State 15 */
  {  24,  32, 0                    }, /*             func_proto shift  32 */
  {  17,  16, 0                    }, /*               arg_type shift  16 */
  {YYNOCODE,0,0}, /* Unused */
  {YYNOCODE,0,0}, /* Unused */
  {   4,  25, 0                    }, /*                  IDENT shift  25 */
  {   5,  22, 0                    }, /*              INTRINSIC shift  22 */
  {  14,  23, 0                    }, /*                 STRUCT shift  23 */
  {YYNOCODE,0,0}, /* Unused */
/* State 16 */
  {   4,  18, 0                    }, /*                  IDENT shift  18 */
  {   1,  17, 0                    }, /*               ASTERISK shift  17 */
/* State 17 */
  {YYNOCODE,0,0}, /* Unused */
/* State 18 */
  {   7,  19, 0                    }, /*                 LPAREN shift  19 */
/* State 19 */
  {  16,  26, 0                    }, /*               arg_list shift  26 */
  {  17,  20, 0                    }, /*               arg_type shift  20 */
  {  18,  29, &yyActionTable[  67] }, /*               argument shift  29 */
  {  10,  70, 0                    }, /*                 RPAREN reduce 22 */
  {   4,  25, 0                    }, /*                  IDENT shift  25 */
  {   5,  22, 0                    }, /*              INTRINSIC shift  22 */
  {  14,  23, 0                    }, /*                 STRUCT shift  23 */
  {YYNOCODE,0,0}, /* Unused */
/* State 20 */
  {   4,  21, 0                    }, /*                  IDENT shift  21 */
  {   1,  17, 0                    }, /*               ASTERISK shift  17 */
/* State 21 */
  {YYNOCODE,0,0}, /* Unused */
/* State 22 */
  {YYNOCODE,0,0}, /* Unused */
/* State 23 */
  {   4,  24, 0                    }, /*                  IDENT shift  24 */
/* State 24 */
  {YYNOCODE,0,0}, /* Unused */
/* State 25 */
  {YYNOCODE,0,0}, /* Unused */
/* State 26 */
  {  10,  27, 0                    }, /*                 RPAREN shift  27 */
/* State 27 */
  {  12,  28, 0                    }, /*                   SEMI shift  28 */
/* State 28 */
  {YYNOCODE,0,0}, /* Unused */
/* State 29 */
  {  10,  69, &yyActionTable[  83] }, /*                 RPAREN reduce 21 */
  {   2,  30, 0                    }, /*                  COMMA shift  30 */
/* State 30 */
  {  16,  31, 0                    }, /*               arg_list shift  31 */
  {  17,  20, 0                    }, /*               arg_type shift  20 */
  {  18,  29, &yyActionTable[  87] }, /*               argument shift  29 */
  {  10,  70, 0                    }, /*                 RPAREN reduce 22 */
  {   4,  25, 0                    }, /*                  IDENT shift  25 */
  {   5,  22, 0                    }, /*              INTRINSIC shift  22 */
  {  14,  23, 0                    }, /*                 STRUCT shift  23 */
  {YYNOCODE,0,0}, /* Unused */
/* State 31 */
  {  10,  68, 0                    }, /*                 RPAREN reduce 20 */
/* State 32 */
  {YYNOCODE,0,0}, /* Unused */
/* State 33 */
  {   4,  25, 0                    }, /*                  IDENT shift  25 */
  {  17,  34, &yyActionTable[  97] }, /*               arg_type shift  34 */
  {  14,  23, 0                    }, /*                 STRUCT shift  23 */
  {   5,  22, 0                    }, /*              INTRINSIC shift  22 */
/* State 34 */
  {   4,  35, 0                    }, /*                  IDENT shift  35 */
  {   1,  17, 0                    }, /*               ASTERISK shift  17 */
/* State 35 */
  {  12,  36, 0                    }, /*                   SEMI shift  36 */
/* State 36 */
  {YYNOCODE,0,0}, /* Unused */
/* State 37 */
  {   4,  38, 0                    }, /*                  IDENT shift  38 */
/* State 38 */
  {   6,  39, 0                    }, /*                 LBRACE shift  39 */
/* State 39 */
  {   4,  25, 0                    }, /*                  IDENT shift  25 */
  {  17,  40, 0                    }, /*               arg_type shift  40 */
  {   5,  22, 0                    }, /*              INTRINSIC shift  22 */
  {YYNOCODE,0,0}, /* Unused */
  {  20,  46, &yyActionTable[ 104] }, /*              field_def shift  46 */
  {  21,  43, &yyActionTable[ 106] }, /*             field_list shift  43 */
  {  14,  23, 0                    }, /*                 STRUCT shift  23 */
  {YYNOCODE,0,0}, /* Unused */
/* State 40 */
  {   4,  41, 0                    }, /*                  IDENT shift  41 */
  {   1,  17, 0                    }, /*               ASTERISK shift  17 */
/* State 41 */
  {  12,  42, 0                    }, /*                   SEMI shift  42 */
/* State 42 */
  {YYNOCODE,0,0}, /* Unused */
/* State 43 */
  {   9,  44, 0                    }, /*                 RBRACE shift  44 */
/* State 44 */
  {  12,  45, 0                    }, /*                   SEMI shift  45 */
/* State 45 */
  {YYNOCODE,0,0}, /* Unused */
/* State 46 */
  {   9,  66, 0                    }, /*                 RBRACE reduce 18 */
  {  17,  40, &yyActionTable[ 119] }, /*               arg_type shift  40 */
  {   4,  25, 0                    }, /*                  IDENT shift  25 */
  {   5,  22, 0                    }, /*              INTRINSIC shift  22 */
  {  20,  46, &yyActionTable[ 121] }, /*              field_def shift  46 */
  {  21,  47, &yyActionTable[ 122] }, /*             field_list shift  47 */
  {  14,  23, 0                    }, /*                 STRUCT shift  23 */
  {YYNOCODE,0,0}, /* Unused */
/* State 47 */
  {   9,  65, 0                    }, /*                 RBRACE reduce 17 */
};

/* The state table contains information needed to look up the correct
** action in the action table, given the current state of the parser.
** Information needed includes:
**
**  +  A pointer to the start of the action hash table in yyActionTable.
**
**  +  A mask used to hash the look-ahead token.  The mask is an integer
**     which is one less than the size of the hash table.  
**
**  +  The default action.  This is the action to take if no entry for
**     the given look-ahead is found in the action hash table.
*/
struct yyStateEntry {
  struct yyActionEntry *hashtbl; /* Start of the hash table in yyActionTable */
  int mask;                      /* Mask used for hashing the look-ahead */
  YYACTIONTYPE actionDefault;    /* Default action if look-ahead not found */
};
static struct yyStateEntry yyStateTable[] = {
  { &yyActionTable[0], 15, 72},
  { &yyActionTable[16], 0, 72},
  { &yyActionTable[17], 15, 72},
  { &yyActionTable[33], 0, 72},
  { &yyActionTable[34], 0, 51},
  { &yyActionTable[35], 0, 52},
  { &yyActionTable[36], 3, 72},
  { &yyActionTable[40], 1, 72},
  { &yyActionTable[42], 3, 72},
  { &yyActionTable[46], 0, 72},
  { &yyActionTable[47], 0, 72},
  { &yyActionTable[48], 0, 72},
  { &yyActionTable[49], 0, 59},
  { &yyActionTable[50], 0, 72},
  { &yyActionTable[51], 0, 60},
  { &yyActionTable[52], 7, 72},
  { &yyActionTable[60], 1, 72},
  { &yyActionTable[62], 0, 53},
  { &yyActionTable[63], 0, 72},
  { &yyActionTable[64], 7, 72},
  { &yyActionTable[72], 1, 72},
  { &yyActionTable[74], 0, 71},
  { &yyActionTable[75], 0, 54},
  { &yyActionTable[76], 0, 72},
  { &yyActionTable[77], 0, 55},
  { &yyActionTable[78], 0, 56},
  { &yyActionTable[79], 0, 72},
  { &yyActionTable[80], 0, 72},
  { &yyActionTable[81], 0, 62},
  { &yyActionTable[82], 1, 72},
  { &yyActionTable[84], 7, 72},
  { &yyActionTable[92], 0, 72},
  { &yyActionTable[93], 0, 61},
  { &yyActionTable[94], 3, 72},
  { &yyActionTable[98], 1, 72},
  { &yyActionTable[100], 0, 72},
  { &yyActionTable[101], 0, 63},
  { &yyActionTable[102], 0, 72},
  { &yyActionTable[103], 0, 72},
  { &yyActionTable[104], 7, 72},
  { &yyActionTable[112], 1, 72},
  { &yyActionTable[114], 0, 72},
  { &yyActionTable[115], 0, 67},
  { &yyActionTable[116], 0, 72},
  { &yyActionTable[117], 0, 72},
  { &yyActionTable[118], 0, 64},
  { &yyActionTable[119], 7, 72},
  { &yyActionTable[127], 0, 72},
};

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int idx;                            /* Index of top element in stack */
  int errcnt;                         /* Shifts left before out of the error */
  struct yyStackEntry *top;           /* Pointer to the top stack element */
  struct yyStackEntry stack[YYSTACKDEPTH];  /* The parser's stack */
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;

/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void php_ffi_parserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}

/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static char *yyTokenName[] = { 
  "$",             "ASTERISK",      "COMMA",         "EQUALS",      
  "IDENT",         "INTRINSIC",     "LBRACE",        "LPAREN",      
  "LSQUARE",       "RBRACE",        "RPAREN",        "RSQUARE",     
  "SEMI",          "STRING",        "STRUCT",        "TYPEDEF",     
  "arg_list",      "arg_type",      "argument",      "error",       
  "field_def",     "field_list",    "func_attribute",  "func_attribute_list",
  "func_proto",    "func_proto_with_attributes",  "optional_func_attribute_list",  "top_item",    
  "top_level",     "top_list",      "type_def",    
};
#define YYTRACE(X) if( yyTraceFILE ) fprintf(yyTraceFILE,"%sReduce [%s].\n",yyTracePrompt,X);
#else
#define YYTRACE(X)
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to php_ffi_parser and php_ffi_parserFree.
*/
void *php_ffi_parserAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (int)sizeof(yyParser) );
  if( pParser ){
    pParser->idx = -1;
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;

  if( pParser->idx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->idx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[pParser->top->major]);
  }
#endif
  yymajor = pParser->top->major;
  yy_destructor( yymajor, &pParser->top->minor);
  pParser->idx--;
  pParser->top--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from php_ffi_parserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void php_ffi_parserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->idx>=0 ) yy_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the look-ahead token.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_parser_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead             /* The look-ahead token */
){
  struct yyStateEntry *pState;   /* Appropriate entry in the state table */
  struct yyActionEntry *pAction; /* Action appropriate for the look-ahead */
 
  /* if( pParser->idx<0 ) return YY_NO_ACTION;  */
  pState = &yyStateTable[pParser->top->stateno];
  if( iLookAhead!=YYNOCODE ){
    pAction = &pState->hashtbl[iLookAhead & pState->mask];
    while( pAction ){
      if( pAction->lookahead==iLookAhead ) return pAction->action;
      pAction = pAction->next;
    }
  }else if( pState->mask!=0 || pState->hashtbl->lookahead!=YYNOCODE ){
    return YY_NO_ACTION;
  }
  return pState->actionDefault;
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yypParser->idx++;
  yypParser->top++;
  if( yypParser->idx>=YYSTACKDEPTH ){
     yypParser->idx--;
     yypParser->top--;
#ifndef NDEBUG
     if( yyTraceFILE ){
       fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
     }
#endif
     while( yypParser->idx>=0 ) yy_pop_parser_stack(yypParser);
     /* Here code is inserted which will execute if the parser
     ** stack every overflows */
     return;
  }
  yypParser->top->stateno = yyNewState;
  yypParser->top->major = yyMajor;
  yypParser->top->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->idx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->idx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->stack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 28, 1 },
  { 29, 2 },
  { 29, 1 },
  { 27, 1 },
  { 27, 1 },
  { 17, 2 },
  { 17, 1 },
  { 17, 2 },
  { 17, 1 },
  { 23, 3 },
  { 23, 1 },
  { 22, 3 },
  { 26, 3 },
  { 25, 2 },
  { 24, 6 },
  { 30, 4 },
  { 30, 6 },
  { 21, 2 },
  { 21, 1 },
  { 20, 3 },
  { 16, 3 },
  { 16, 1 },
  { 16, 0 },
  { 18, 2 },
};

static void yy_accept(yyParser *  php_ffi_parserANSIARGDECL);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
  php_ffi_parserANSIARGDECL
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  struct yyStackEntry *yymsp;     /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  yymsp = yypParser->top;
  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **     YYTRACE("<text of the rule>");
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0:
        YYTRACE("top_level ::= top_list")
        /* No destructor defined for top_list */
        break;
      case 1:
        YYTRACE("top_list ::= top_item top_list")
        /* No destructor defined for top_item */
        /* No destructor defined for top_list */
        break;
      case 2:
        YYTRACE("top_list ::= top_item")
        /* No destructor defined for top_item */
        break;
      case 3:
        YYTRACE("top_item ::= func_proto_with_attributes")
        /* No destructor defined for func_proto_with_attributes */
        break;
      case 4:
        YYTRACE("top_item ::= type_def")
        /* No destructor defined for type_def */
        break;
      case 5:
        YYTRACE("arg_type ::= arg_type ASTERISK")
#line 257 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{ yygotominor.yy0 = yymsp[-1].minor.yy0; yygotominor.yy0.type.ptr_levels++; }
#line 922 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        /* No destructor defined for ASTERISK */
        break;
      case 6:
        YYTRACE("arg_type ::= INTRINSIC")
#line 258 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{ yygotominor.yy0 = yymsp[0].minor.yy0; yygotominor.yy0.type.ptr_levels = 0;	}
#line 929 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        break;
      case 7:
        YYTRACE("arg_type ::= STRUCT IDENT")
#line 259 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{
   	yygotominor.yy0.type.intrinsic_type = FFI_TYPE_STRUCT;
	yygotominor.yy0.type.struct_name = yymsp[0].minor.yy0.ident;
   	yygotominor.yy0.type.ptr_levels = 0;
}
#line 939 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        /* No destructor defined for STRUCT */
        break;
      case 8:
        YYTRACE("arg_type ::= IDENT")
#line 265 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{ /* TODO */ }
#line 946 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        break;
      case 9:
        YYTRACE("func_attribute_list ::= func_attribute COMMA func_attribute_list")
        /* No destructor defined for func_attribute */
        /* No destructor defined for COMMA */
        /* No destructor defined for func_attribute_list */
        break;
      case 10:
        YYTRACE("func_attribute_list ::= func_attribute")
        /* No destructor defined for func_attribute */
        break;
      case 11:
        YYTRACE("func_attribute ::= IDENT EQUALS STRING")
#line 270 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{
	if (IDENT_EQUALS("lib", yymsp[-2].minor.yy0.ident)) {
		ctx->libname = yymsp[0].minor.yy0.ident;
	} else {
		CTX_TSRMLS_FETCH();
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported attribute %s", get_ident_string(yymsp[-2].minor.yy0.ident));
	}
}
#line 969 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        /* No destructor defined for EQUALS */
        break;
      case 12:
        YYTRACE("optional_func_attribute_list ::= LSQUARE func_attribute_list RSQUARE")
        /* No destructor defined for LSQUARE */
        /* No destructor defined for func_attribute_list */
        /* No destructor defined for RSQUARE */
        break;
      case 13:
        YYTRACE("func_proto_with_attributes ::= optional_func_attribute_list func_proto")
        /* No destructor defined for optional_func_attribute_list */
        break;
      case 14:
        YYTRACE("func_proto ::= arg_type IDENT LPAREN arg_list RPAREN SEMI")
#line 283 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{
	yygotominor.yy0.func = register_func(ctx, yymsp[-5].minor.yy0.type, yymsp[-4].minor.yy0.ident);
	ctx->n_args = 0;
}
#line 989 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        /* No destructor defined for LPAREN */
        /* No destructor defined for arg_list */
        /* No destructor defined for RPAREN */
        /* No destructor defined for SEMI */
        break;
      case 15:
        YYTRACE("type_def ::= TYPEDEF arg_type IDENT SEMI")
#line 289 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{
	CTX_TSRMLS_FETCH();
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "aliasing typedefs are not yet implemented");
}
#line 1002 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        /* No destructor defined for TYPEDEF */
        /* No destructor defined for SEMI */
        break;
      case 16:
        YYTRACE("type_def ::= STRUCT IDENT LBRACE field_list RBRACE SEMI")
#line 294 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{
	register_type(ctx, yymsp[-4].minor.yy0.ident);	
	ctx->n_args = 0;
}
#line 1013 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        /* No destructor defined for STRUCT */
        /* No destructor defined for LBRACE */
        /* No destructor defined for field_list */
        /* No destructor defined for RBRACE */
        /* No destructor defined for SEMI */
        break;
      case 17:
        YYTRACE("field_list ::= field_def field_list")
        /* No destructor defined for field_def */
        /* No destructor defined for field_list */
        break;
      case 18:
        YYTRACE("field_list ::= field_def")
        /* No destructor defined for field_def */
        break;
      case 19:
        YYTRACE("field_def ::= arg_type IDENT SEMI")
#line 302 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{
	add_arg(ctx, yymsp[-2].minor.yy0.type, yymsp[-1].minor.yy0.ident);
}
#line 1035 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        /* No destructor defined for SEMI */
        break;
      case 20:
        YYTRACE("arg_list ::= argument COMMA arg_list")
        /* No destructor defined for argument */
        /* No destructor defined for COMMA */
        /* No destructor defined for arg_list */
        break;
      case 21:
        YYTRACE("arg_list ::= argument")
        /* No destructor defined for argument */
        break;
      case 22:
        YYTRACE("arg_list ::=")
        break;
      case 23:
        YYTRACE("argument ::= arg_type IDENT")
#line 310 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
{
	add_arg(ctx, yymsp[-1].minor.yy0.type, yymsp[0].minor.yy0.ident);
}
#line 1057 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->idx -= yysize;
  yypParser->top -= yysize;
  yyact = yy_find_parser_action(yypParser,yygoto);
  if( yyact < YYNSTATE ){
    yy_shift(yypParser,yyact,yygoto,&yygotominor);
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser php_ffi_parserARGDECL);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
  php_ffi_parserANSIARGDECL              /* Extra arguments (if any) */
){
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->idx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 5 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
 ctx->failed = 1; printf("FAIL: parser failed - %d errors\n", ctx->errors); 
#line 1089 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
  php_ffi_parserANSIARGDECL               /* Extra arguments (if any) */
){
#define TOKEN (yyminor.yy0)
#line 6 "/home/wez/src/php/php5/ext/ffi/ffi_parser.y"
 ctx->errors++; printf("SYNTAX: entering error recovery near token %s\n", php_ffi_get_token_string(yymajor, TOKEN)); 
#line 1104 "/home/wez/src/php/php5/ext/ffi/ffi_parser.c"
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
  php_ffi_parserANSIARGDECL              /* Extra arguments (if any) */
){
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->idx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "php_ffi_parserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void php_ffi_parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  php_ffi_parserTOKENTYPE yyminor       /* The value for the token */
  php_ffi_parserANSIARGDECL
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->idx<0 ){
    if( yymajor==0 ) return;
    yypParser->idx = 0;
    yypParser->errcnt = -1;
    yypParser->top = &yypParser->stack[0];
    yypParser->top->stateno = 0;
    yypParser->top->major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_parser_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->errcnt--;
      if( yyendofinput && yypParser->idx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE php_ffi_parserARGDECL);
    }else if( yyact == YY_ERROR_ACTION ){
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->errcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion php_ffi_parserARGDECL);
      }
      if( yypParser->top->major==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->idx >= 0 &&
          yypParser->top->major != YYERRORSYMBOL &&
          (yyact = yy_find_parser_action(yypParser,YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->idx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser php_ffi_parserARGDECL);
          yymajor = YYNOCODE;
        }else if( yypParser->top->major!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->errcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->errcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion php_ffi_parserARGDECL);
      }
      yypParser->errcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser php_ffi_parserARGDECL);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser php_ffi_parserARGDECL);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->idx>=0 );
  return;
}
