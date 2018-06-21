/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     I32 = 258,
     I64 = 259,
     U32 = 260,
     U64 = 261,
     T_INT = 262,
     NAME = 263,
     IP = 264,
     FUNCTION = 265,
     IF = 266,
     THEN = 267,
     ELSE = 268,
     FOR = 269,
     RETURN = 270,
     DEF_INPUT = 271,
     DEF_ROLE = 272,
     DEF_PORT = 273,
     DEF_OT_PORT = 274,
     DEF_IP = 275,
     DEF_START = 276,
     GARBLER = 277,
     EVALUATOR = 278,
     CMP = 279,
     S_START = 280,
     S_END = 281,
     UNEG = 282,
     UMINUS = 283
   };
#endif
/* Tokens.  */
#define I32 258
#define I64 259
#define U32 260
#define U64 261
#define T_INT 262
#define NAME 263
#define IP 264
#define FUNCTION 265
#define IF 266
#define THEN 267
#define ELSE 268
#define FOR 269
#define RETURN 270
#define DEF_INPUT 271
#define DEF_ROLE 272
#define DEF_PORT 273
#define DEF_OT_PORT 274
#define DEF_IP 275
#define DEF_START 276
#define GARBLER 277
#define EVALUATOR 278
#define CMP 279
#define S_START 280
#define S_END 281
#define UNEG 282
#define UMINUS 283




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 22 "/Users/xtan/xcode-workspace/gash/gash/parser.ypp"
{
    int32_t yy_val_i32;
    int64_t yy_val_i64;
    uint32_t yy_val_u32;
    uint64_t yy_val_u64;
    uint32_t yy_cmp;
    uint32_t yy_intlen;
    Ast* yy_ast;
    Symbol* yy_sym;
    Vardef* yy_vardef;
    Scope* yy_scope;
    char* yy_ip;
    RoleType yy_role;
}
/* Line 1529 of yacc.c.  */
#line 120 "/Users/xtan/xcode-workspace/gash/gash/parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;
