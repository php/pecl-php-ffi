/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 7 "/home/wez.net/src/pecl-ffi/ffi_parser.y"

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
#line 93 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
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
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    php_ffi_parserARG_SDECL     A static variable declaration for the %extra_argument
**    php_ffi_parserARG_PDECL     A parameter declaration for the %extra_argument
**    php_ffi_parserARG_STORE     Code to store %extra_argument into yypParser
**    php_ffi_parserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 35
#define YYACTIONTYPE unsigned char
#define php_ffi_parserTOKENTYPE  php_ffi_tokentype 
typedef union {
  int yyinit;
  php_ffi_parserTOKENTYPE yy0;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define php_ffi_parserARG_SDECL  struct php_ffi_def_context *ctx ;
#define php_ffi_parserARG_PDECL , struct php_ffi_def_context *ctx 
#define php_ffi_parserARG_FETCH  struct php_ffi_def_context *ctx  = yypParser->ctx 
#define php_ffi_parserARG_STORE yypParser->ctx  = ctx 
#define YYNSTATE 60
#define YYNRULE 29
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };


/* Next are the tables used to determine what action to take based on the
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
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    90,   15,    1,   40,   41,   42,   43,   32,   29,   33,
 /*    10 */    39,    1,   40,   41,   42,   43,   55,   29,   33,   47,
 /*    20 */    16,    9,   17,   48,   13,    9,   18,   48,   45,   23,
 /*    30 */    21,   11,   21,   14,   19,    3,   50,    3,   28,   25,
 /*    40 */    28,   53,   28,   35,   27,   45,   27,   57,   27,   46,
 /*    50 */    16,   30,   33,   37,   60,    8,   44,   10,   20,    2,
 /*    60 */    49,   22,   24,    7,   51,    4,   91,   26,   34,   52,
 /*    70 */    54,    5,   12,   91,   91,   31,    6,   58,   36,   56,
 /*    80 */    38,   59,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    18,   19,   20,   21,   22,   23,   24,   29,   30,   27,
 /*    10 */    19,   20,   21,   22,   23,   24,   29,   30,   27,   25,
 /*    20 */    26,    2,    3,    4,    5,    2,    3,    4,    1,   27,
 /*    30 */    27,   12,   27,   14,   31,   32,   31,   32,   27,   28,
 /*    40 */    27,   28,   27,   28,   33,    1,   33,   22,   33,   25,
 /*    50 */    26,    4,   27,   27,    0,    4,    1,    4,   16,   15,
 /*    60 */     8,    4,    4,   13,    8,    6,   34,    7,    4,    8,
 /*    70 */     4,    9,    9,   34,   34,   10,    6,    8,    7,   11,
 /*    80 */     4,    8,
};
#define YY_SHIFT_USE_DFLT (-1)
#define YY_SHIFT_MAX 38
static const signed char yy_shift_ofst[] = {
 /*     0 */    19,   19,   23,   23,   23,   23,   23,   23,   44,   27,
 /*    10 */    27,   47,   47,   23,   23,   54,   55,   51,   53,   42,
 /*    20 */    52,   57,   56,   58,   59,   60,   61,   62,   66,   63,
 /*    30 */    65,   68,   50,   64,   70,   71,   69,   76,   73,
};
#define YY_REDUCE_USE_DFLT (-23)
#define YY_REDUCE_MAX 14
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -18,   -9,    3,    5,   11,   13,   15,   25,   -6,   24,
 /*    10 */    -6,  -22,  -13,    2,   26,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    89,   62,   89,   83,   87,   87,   87,   89,   67,   67,
 /*    10 */    67,   89,   89,   89,   89,   89,   68,   89,   89,   89,
 /*    20 */    89,   89,   89,   89,   89,   89,   89,   86,   89,   76,
 /*    30 */    89,   89,   89,   89,   89,   89,   89,   89,   89,   61,
 /*    40 */    63,   64,   65,   66,   70,   69,   71,   72,   73,   81,
 /*    50 */    82,   84,   74,   85,   88,   75,   77,   78,   79,   80,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
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
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  php_ffi_parserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
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
void php_ffi_parserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "ASTERISK",      "INTRINSIC",     "STRUCT",      
  "IDENT",         "CALLBACK",      "LPAREN",        "RPAREN",      
  "SEMI",          "COMMA",         "EQUALS",        "STRING",      
  "LSQUARE",       "RSQUARE",       "TYPEDEF",       "LBRACE",      
  "RBRACE",        "error",         "top_level",     "top_list",    
  "top_item",      "func_proto_with_attributes",  "func_proto",    "type_def",    
  "callback_def",  "optional_asterisk_list",  "asterisk_list",  "arg_type",    
  "arg_list",      "func_attribute_list",  "func_attribute",  "field_list",  
  "field_def",     "argument",    
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "top_level ::= top_list",
 /*   1 */ "top_list ::= top_item top_list",
 /*   2 */ "top_list ::= top_item",
 /*   3 */ "top_item ::= func_proto_with_attributes",
 /*   4 */ "top_item ::= func_proto",
 /*   5 */ "top_item ::= type_def",
 /*   6 */ "top_item ::= callback_def",
 /*   7 */ "optional_asterisk_list ::=",
 /*   8 */ "optional_asterisk_list ::= asterisk_list",
 /*   9 */ "asterisk_list ::= ASTERISK",
 /*  10 */ "asterisk_list ::= asterisk_list ASTERISK",
 /*  11 */ "arg_type ::= INTRINSIC optional_asterisk_list",
 /*  12 */ "arg_type ::= STRUCT IDENT optional_asterisk_list",
 /*  13 */ "arg_type ::= IDENT",
 /*  14 */ "callback_def ::= CALLBACK arg_type IDENT LPAREN arg_list RPAREN SEMI",
 /*  15 */ "func_attribute_list ::= func_attribute COMMA func_attribute_list",
 /*  16 */ "func_attribute_list ::= func_attribute",
 /*  17 */ "func_attribute ::= IDENT EQUALS STRING",
 /*  18 */ "func_proto_with_attributes ::= LSQUARE func_attribute_list RSQUARE func_proto",
 /*  19 */ "func_proto ::= arg_type IDENT LPAREN arg_list RPAREN SEMI",
 /*  20 */ "type_def ::= TYPEDEF arg_type IDENT SEMI",
 /*  21 */ "type_def ::= STRUCT IDENT LBRACE field_list RBRACE SEMI",
 /*  22 */ "field_list ::= field_def field_list",
 /*  23 */ "field_list ::= field_def",
 /*  24 */ "field_def ::= arg_type IDENT SEMI",
 /*  25 */ "arg_list ::= argument COMMA arg_list",
 /*  26 */ "arg_list ::= argument",
 /*  27 */ "arg_list ::=",
 /*  28 */ "argument ::= arg_type IDENT",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
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
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  php_ffi_parserARG_FETCH;
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
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
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
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int php_ffi_parserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

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
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
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
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
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
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_MAX ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_MAX );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_SZ_ACTTAB );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   php_ffi_parserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   php_ffi_parserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
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
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 18, 1 },
  { 19, 2 },
  { 19, 1 },
  { 20, 1 },
  { 20, 1 },
  { 20, 1 },
  { 20, 1 },
  { 25, 0 },
  { 25, 1 },
  { 26, 1 },
  { 26, 2 },
  { 27, 2 },
  { 27, 3 },
  { 27, 1 },
  { 24, 7 },
  { 29, 3 },
  { 29, 1 },
  { 30, 3 },
  { 21, 4 },
  { 22, 6 },
  { 23, 4 },
  { 23, 6 },
  { 31, 2 },
  { 31, 1 },
  { 32, 3 },
  { 28, 3 },
  { 28, 1 },
  { 28, 0 },
  { 33, 2 },
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
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* top_level ::= top_list */
      case 1: /* top_list ::= top_item top_list */
      case 2: /* top_list ::= top_item */
      case 3: /* top_item ::= func_proto_with_attributes */
      case 4: /* top_item ::= func_proto */
      case 5: /* top_item ::= type_def */
      case 6: /* top_item ::= callback_def */
      case 15: /* func_attribute_list ::= func_attribute COMMA func_attribute_list */
      case 16: /* func_attribute_list ::= func_attribute */
      case 18: /* func_proto_with_attributes ::= LSQUARE func_attribute_list RSQUARE func_proto */
      case 22: /* field_list ::= field_def field_list */
      case 23: /* field_list ::= field_def */
      case 25: /* arg_list ::= argument COMMA arg_list */
      case 26: /* arg_list ::= argument */
      case 27: /* arg_list ::= */
#line 93 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
}
#line 802 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 7: /* optional_asterisk_list ::= */
#line 102 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{ yygotominor.yy0.type.ptr_levels = 0; }
#line 807 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 8: /* optional_asterisk_list ::= asterisk_list */
#line 103 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{ yygotominor.yy0 = yymsp[0].minor.yy0; }
#line 812 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 9: /* asterisk_list ::= ASTERISK */
#line 105 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{ yygotominor.yy0.type.ptr_levels = 1; }
#line 817 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 10: /* asterisk_list ::= asterisk_list ASTERISK */
#line 106 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{ yygotominor.yy0 = yymsp[-1].minor.yy0; yygotominor.yy0.type.ptr_levels++; }
#line 822 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 11: /* arg_type ::= INTRINSIC optional_asterisk_list */
#line 108 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{ yygotominor.yy0 = yymsp[-1].minor.yy0; yygotominor.yy0.type.ptr_levels = yymsp[0].minor.yy0.type.ptr_levels; }
#line 827 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 12: /* arg_type ::= STRUCT IDENT optional_asterisk_list */
#line 109 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
   	yygotominor.yy0.type.intrinsic_type = FFI_TYPE_STRUCT;
	yygotominor.yy0.type.struct_name = yymsp[-1].minor.yy0.ident;
   	yygotominor.yy0.type.ptr_levels = yymsp[0].minor.yy0.type.ptr_levels;
}
#line 836 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 13: /* arg_type ::= IDENT */
#line 115 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	CTX_TSRMLS_FETCH();
	/* TODO: lookup ident (including callback types) */
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unrecognized type name `%s'", get_ident_string(yymsp[0].minor.yy0.ident));	
}
#line 845 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 14: /* callback_def ::= CALLBACK arg_type IDENT LPAREN arg_list RPAREN SEMI */
#line 123 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	CTX_TSRMLS_FETCH();
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "callback support not complete");	
}
#line 853 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 17: /* func_attribute ::= IDENT EQUALS STRING */
#line 131 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	if (IDENT_EQUALS("lib", yymsp[-2].minor.yy0.ident)) {
		ctx->libname = yymsp[0].minor.yy0.ident;
	} else {
		CTX_TSRMLS_FETCH();
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported attribute %s", get_ident_string(yymsp[-2].minor.yy0.ident));
	}
}
#line 865 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 19: /* func_proto ::= arg_type IDENT LPAREN arg_list RPAREN SEMI */
#line 142 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	yygotominor.yy0.func = php_ffi_parser_register_func(ctx, yymsp[-5].minor.yy0.type, yymsp[-4].minor.yy0.ident);
	ctx->n_args = 0;
}
#line 873 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 20: /* type_def ::= TYPEDEF arg_type IDENT SEMI */
#line 147 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	CTX_TSRMLS_FETCH();
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "aliasing typedefs are not yet implemented");
}
#line 881 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 21: /* type_def ::= STRUCT IDENT LBRACE field_list RBRACE SEMI */
#line 152 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	php_ffi_parser_register_type(ctx, yymsp[-4].minor.yy0.ident);	
	ctx->n_args = 0;
}
#line 889 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 24: /* field_def ::= arg_type IDENT SEMI */
#line 160 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	php_ffi_parser_add_arg(ctx, yymsp[-2].minor.yy0.type, yymsp[-1].minor.yy0.ident);
}
#line 896 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
      case 28: /* argument ::= arg_type IDENT */
#line 168 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
{
	php_ffi_parser_add_arg(ctx, yymsp[-1].minor.yy0.type, yymsp[0].minor.yy0.ident);
}
#line 903 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
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
#line 5 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
 ctx->failed = 1; printf("FAIL: parser failed - %d errors\n", ctx->errors); 
#line 950 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
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
#line 6 "/home/wez.net/src/pecl-ffi/ffi_parser.y"
 ctx->errors++; printf("SYNTAX: entering error recovery near token %s\n", php_ffi_get_token_string(yymajor, TOKEN)); 
#line 966 "/home/wez.net/src/pecl-ffi/ffi_parser.c"
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
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
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
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
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
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
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
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
