/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
#line 7 "..\\pecl\\ffi\\ffi_parser.y"

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
#if ZEND_DEBUG
			sprintf(tokbuf, "token: %s", php_ffi_parserTokenName(major));
#else
			sprintf(tokbuf, "token: ???");
#endif
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





#line 255 "..\\pecl\\ffi\\ffi_parser.c"
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
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
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
**    php_ffi_parserARG_SDECL     A static variable declaration for the %extra_argument
**    php_ffi_parserARG_PDECL     A parameter declaration for the %extra_argument
**    php_ffi_parserARG_STORE     Code to store %extra_argument into yypParser
**    php_ffi_parserARG_FETCH     Code to extract %extra_argument from yypParser
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
#define php_ffi_parserARG_SDECL  struct php_ffi_def_context *ctx ;
#define php_ffi_parserARG_PDECL , struct php_ffi_def_context *ctx 
#define php_ffi_parserARG_FETCH  struct php_ffi_def_context *ctx  = yypParser->ctx 
#define php_ffi_parserARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE 48
#define YYNRULE 24
#define YYERRORSYMBOL 19
#define YYERRSYMDT yy63
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static YYACTIONTYPE yy_action[] = {
 /*     0 */     4,   15,    2,   73,    1,    5,    4,   15,    2,   50,
 /*    10 */     3,    5,   25,   22,   16,   25,   22,    6,   70,   10,
 /*    20 */    66,   32,   23,   37,   33,   23,   31,   20,   29,   25,
 /*    30 */    22,   40,   57,   17,   46,   43,   35,    6,   30,   23,
 /*    40 */    26,   20,   29,   37,   33,   40,   69,   17,   46,   47,
 /*    50 */    21,   17,   17,   48,   18,   41,    7,   13,    8,    7,
 /*    60 */     9,   49,   11,   44,   19,   12,   14,   58,   24,   27,
 /*    70 */    28,   68,   34,   36,   38,   39,   42,   45,   65,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    25,   26,   27,   28,   29,   30,   25,   26,   27,    0,
 /*    10 */    29,   30,    4,    5,   17,    4,    5,    8,   10,    4,
 /*    20 */     9,   24,   14,   14,   15,   14,   16,   17,   18,    4,
 /*    30 */     5,   17,   11,    1,   20,   21,    4,    8,    2,   14,
 /*    40 */    16,   17,   18,   14,   15,   17,   10,    1,   20,   21,
 /*    50 */     4,    1,    1,    0,    4,    4,   22,   23,    2,   22,
 /*    60 */    23,    0,    3,    9,    7,   13,   11,   11,    4,   10,
 /*    70 */    12,   10,   17,   12,    4,    6,   12,   12,    9,
};
#define YY_SHIFT_USE_DFLT (-1)
static signed char yy_shift_ofst[] = {
 /*     0 */    29,   53,    9,   61,   -1,   -1,   15,   56,   15,   21,
 /*    10 */    59,   52,   -1,   55,   -1,   25,   50,   -1,   57,    8,
 /*    20 */    46,   -1,   -1,   64,   -1,   -1,   59,   58,   -1,   36,
 /*    30 */     8,   61,   -1,   25,   32,   61,   -1,   70,   69,   25,
 /*    40 */    51,   64,   -1,   54,   65,   -1,   11,   69,
};
#define YY_REDUCE_USE_DFLT (-26)
static signed char yy_reduce_ofst[] = {
 /*     0 */   -25,  -26,  -19,  -26,  -26,  -26,   34,  -26,   37,  -26,
 /*    10 */   -26,  -26,  -26,  -26,  -26,   -3,  -26,  -26,  -26,   24,
 /*    20 */   -26,  -26,  -26,  -26,  -26,  -26,  -26,  -26,  -26,  -26,
 /*    30 */    10,  -26,  -26,   55,  -26,  -26,  -26,  -26,  -26,   14,
 /*    40 */   -26,  -26,  -26,  -26,  -26,  -26,   28,  -26,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */    72,   72,   72,   72,   51,   52,   72,   72,   72,   72,
 /*    10 */    72,   72,   59,   72,   60,   72,   72,   53,   72,   72,
 /*    20 */    72,   71,   54,   72,   55,   56,   72,   72,   62,   72,
 /*    30 */    72,   72,   61,   72,   72,   72,   63,   72,   72,   72,
 /*    40 */    72,   72,   67,   72,   72,   64,   72,   72,
};
#define YY_SZ_ACTTAB (sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

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
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  yyStackEntry *yytop;          /* Pointer to the top stack element */
  php_ffi_parserARG_SDECL                /* A place to hold %extra_argument */
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static const char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
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
void php_ffi_parserTrace(FILE *TraceFILE, const char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *yyTokenName[] = { 
  "$",             "ASTERISK",      "COMMA",         "EQUALS",      
  "IDENT",         "INTRINSIC",     "LBRACE",        "LPAREN",      
  "LSQUARE",       "RBRACE",        "RPAREN",        "RSQUARE",     
  "SEMI",          "STRING",        "STRUCT",        "TYPEDEF",     
  "arg_list",      "arg_type",      "argument",      "error",       
  "field_def",     "field_list",    "func_attribute",  "func_attribute_list",
  "func_proto",    "func_proto_with_attributes",  "optional_func_attribute_list",  "top_item",    
  "top_level",     "top_list",      "type_def",    
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *yyRuleName[] = {
 /*   0 */ "top_level ::= top_list",
 /*   1 */ "top_list ::= top_item top_list",
 /*   2 */ "top_list ::= top_item",
 /*   3 */ "top_item ::= func_proto_with_attributes",
 /*   4 */ "top_item ::= type_def",
 /*   5 */ "arg_type ::= arg_type ASTERISK",
 /*   6 */ "arg_type ::= INTRINSIC",
 /*   7 */ "arg_type ::= STRUCT IDENT",
 /*   8 */ "arg_type ::= IDENT",
 /*   9 */ "func_attribute_list ::= func_attribute COMMA func_attribute_list",
 /*  10 */ "func_attribute_list ::= func_attribute",
 /*  11 */ "func_attribute ::= IDENT EQUALS STRING",
 /*  12 */ "optional_func_attribute_list ::= LSQUARE func_attribute_list RSQUARE",
 /*  13 */ "func_proto_with_attributes ::= optional_func_attribute_list func_proto",
 /*  14 */ "func_proto ::= arg_type IDENT LPAREN arg_list RPAREN SEMI",
 /*  15 */ "type_def ::= TYPEDEF arg_type IDENT SEMI",
 /*  16 */ "type_def ::= STRUCT IDENT LBRACE field_list RBRACE SEMI",
 /*  17 */ "field_list ::= field_def field_list",
 /*  18 */ "field_list ::= field_def",
 /*  19 */ "field_def ::= arg_type IDENT SEMI",
 /*  20 */ "arg_list ::= argument COMMA arg_list",
 /*  21 */ "arg_list ::= argument",
 /*  22 */ "arg_list ::=",
 /*  23 */ "argument ::= arg_type IDENT",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
const char *php_ffi_parserTokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && (unsigned int) tokenType<(sizeof(yyTokenName)/sizeof(yyTokenName[0])) ){
    return yyTokenName[tokenType];
  }else{
    return "Unknown";
  }
#else
  return "";
#endif
}

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
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
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

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[pParser->yytop->major]);
  }
#endif
  yymajor = pParser->yytop->major;
  yy_destructor( yymajor, &pParser->yytop->minor);
  pParser->yyidx--;
  pParser->yytop--;
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
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
 
  /* if( pParser->yyidx<0 ) return YY_NO_ACTION;  */
  i = yy_shift_ofst[pParser->yytop->stateno];
  if( i==YY_SHIFT_USE_DFLT ){
    return yy_default[pParser->yytop->stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || (unsigned int)i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
    int iFallback;            /* Fallback token */
    if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
           && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
           yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
      }
#endif
      return yy_find_shift_action(pParser, iFallback);
    }
#endif
    return yy_default[pParser->yytop->stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
 
  i = yy_reduce_ofst[pParser->yytop->stateno];
  if( i==YY_REDUCE_USE_DFLT ){
    return yy_default[pParser->yytop->stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || (unsigned int)i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[pParser->yytop->stateno];
  }else{
    return yy_action[i];
  }
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
  yypParser->yyidx++;
  yypParser->yytop++;
  if( yypParser->yyidx>=YYSTACKDEPTH ){
     php_ffi_parserARG_FETCH;
     yypParser->yyidx--;
     yypParser->yytop--;
#ifndef NDEBUG
     if( yyTraceFILE ){
       fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
     }
#endif
     while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
     /* Here code is inserted which will execute if the parser
     ** stack every overflows */
     php_ffi_parserARG_STORE; /* Suppress warning about unused %extra_argument var */
     return;
  }
  yypParser->yytop->stateno = yyNewState;
  yypParser->yytop->major = yyMajor;
  yypParser->yytop->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
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

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  php_ffi_parserARG_FETCH;
  yymsp = yypParser->yytop;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && (unsigned int)yyruleno<sizeof(yyRuleName)/sizeof(yyRuleName[0]) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0:
        /* No destructor defined for top_list */
        break;
      case 1:
        /* No destructor defined for top_item */
        /* No destructor defined for top_list */
        break;
      case 2:
        /* No destructor defined for top_item */
        break;
      case 3:
        /* No destructor defined for func_proto_with_attributes */
        break;
      case 4:
        /* No destructor defined for type_def */
        break;
      case 5:
#line 261 "..\\pecl\\ffi\\ffi_parser.y"
{ yygotominor.yy0 = yymsp[-1].minor.yy0; yygotominor.yy0.type.ptr_levels++; }
#line 847 "..\\pecl\\ffi\\ffi_parser.c"
        /* No destructor defined for ASTERISK */
        break;
      case 6:
#line 262 "..\\pecl\\ffi\\ffi_parser.y"
{ yygotominor.yy0 = yymsp[0].minor.yy0; yygotominor.yy0.type.ptr_levels = 0;	}
#line 853 "..\\pecl\\ffi\\ffi_parser.c"
        break;
      case 7:
#line 263 "..\\pecl\\ffi\\ffi_parser.y"
{
   	yygotominor.yy0.type.intrinsic_type = FFI_TYPE_STRUCT;
	yygotominor.yy0.type.struct_name = yymsp[0].minor.yy0.ident;
   	yygotominor.yy0.type.ptr_levels = 0;
}
#line 862 "..\\pecl\\ffi\\ffi_parser.c"
        /* No destructor defined for STRUCT */
        break;
      case 8:
#line 269 "..\\pecl\\ffi\\ffi_parser.y"
{ /* TODO */ }
#line 868 "..\\pecl\\ffi\\ffi_parser.c"
        break;
      case 9:
        /* No destructor defined for func_attribute */
        /* No destructor defined for COMMA */
        /* No destructor defined for func_attribute_list */
        break;
      case 10:
        /* No destructor defined for func_attribute */
        break;
      case 11:
#line 274 "..\\pecl\\ffi\\ffi_parser.y"
{
	if (IDENT_EQUALS("lib", yymsp[-2].minor.yy0.ident)) {
		ctx->libname = yymsp[0].minor.yy0.ident;
	} else {
		CTX_TSRMLS_FETCH();
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported attribute %s", get_ident_string(yymsp[-2].minor.yy0.ident));
	}
}
#line 888 "..\\pecl\\ffi\\ffi_parser.c"
        /* No destructor defined for EQUALS */
        break;
      case 12:
        /* No destructor defined for LSQUARE */
        /* No destructor defined for func_attribute_list */
        /* No destructor defined for RSQUARE */
        break;
      case 13:
        /* No destructor defined for optional_func_attribute_list */
        break;
      case 14:
#line 287 "..\\pecl\\ffi\\ffi_parser.y"
{
	yygotominor.yy0.func = register_func(ctx, yymsp[-5].minor.yy0.type, yymsp[-4].minor.yy0.ident);
	ctx->n_args = 0;
}
#line 905 "..\\pecl\\ffi\\ffi_parser.c"
        /* No destructor defined for LPAREN */
        /* No destructor defined for arg_list */
        /* No destructor defined for RPAREN */
        /* No destructor defined for SEMI */
        break;
      case 15:
#line 293 "..\\pecl\\ffi\\ffi_parser.y"
{
	CTX_TSRMLS_FETCH();
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "aliasing typedefs are not yet implemented");
}
#line 917 "..\\pecl\\ffi\\ffi_parser.c"
        /* No destructor defined for TYPEDEF */
        /* No destructor defined for SEMI */
        break;
      case 16:
#line 298 "..\\pecl\\ffi\\ffi_parser.y"
{
	register_type(ctx, yymsp[-4].minor.yy0.ident);	
	ctx->n_args = 0;
}
#line 927 "..\\pecl\\ffi\\ffi_parser.c"
        /* No destructor defined for STRUCT */
        /* No destructor defined for LBRACE */
        /* No destructor defined for field_list */
        /* No destructor defined for RBRACE */
        /* No destructor defined for SEMI */
        break;
      case 17:
        /* No destructor defined for field_def */
        /* No destructor defined for field_list */
        break;
      case 18:
        /* No destructor defined for field_def */
        break;
      case 19:
#line 306 "..\\pecl\\ffi\\ffi_parser.y"
{
	add_arg(ctx, yymsp[-2].minor.yy0.type, yymsp[-1].minor.yy0.ident);
}
#line 946 "..\\pecl\\ffi\\ffi_parser.c"
        /* No destructor defined for SEMI */
        break;
      case 20:
        /* No destructor defined for argument */
        /* No destructor defined for COMMA */
        /* No destructor defined for arg_list */
        break;
      case 21:
        /* No destructor defined for argument */
        break;
      case 22:
        break;
      case 23:
#line 314 "..\\pecl\\ffi\\ffi_parser.y"
{
	add_arg(ctx, yymsp[-1].minor.yy0.type, yymsp[0].minor.yy0.ident);
}
#line 964 "..\\pecl\\ffi\\ffi_parser.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yypParser->yytop -= yysize;
  yyact = yy_find_reduce_action(yypParser,yygoto);
  if( yyact < YYNSTATE ){
    yy_shift(yypParser,yyact,yygoto,&yygotominor);
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  php_ffi_parserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 5 "..\\pecl\\ffi\\ffi_parser.y"
 ctx->failed = 1; printf("FAIL: parser failed - %d errors\n", ctx->errors); 
#line 996 "..\\pecl\\ffi\\ffi_parser.c"
  php_ffi_parserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  php_ffi_parserARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 6 "..\\pecl\\ffi\\ffi_parser.y"
 ctx->errors++; printf("SYNTAX: entering error recovery near token %s\n", php_ffi_get_token_string(yymajor, TOKEN)); 
#line 1012 "..\\pecl\\ffi\\ffi_parser.c"
  php_ffi_parserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  php_ffi_parserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  php_ffi_parserARG_STORE; /* Suppress warning about unused %extra_argument variable */
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
  php_ffi_parserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
    if( yymajor==0 ) return;
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yytop = &yypParser->yystack[0];
    yypParser->yytop->stateno = 0;
    yypParser->yytop->major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  php_ffi_parserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      if( yyendofinput && yypParser->yyidx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
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
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      if( yypParser->yytop->major==YYERRORSYMBOL || yyerrorhit ){
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
          yypParser->yyidx >= 0 &&
          yypParser->yytop->major != YYERRORSYMBOL &&
          (yyact = yy_find_shift_action(yypParser,YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yypParser->yytop->major!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
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
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}

/*
Local variables:
tab-width: 4
c-basic-offset: 4
End:
vim600: noet sw=4 ts=4 fdm=marker
vim<600: noet sw=4 ts=4
*/
