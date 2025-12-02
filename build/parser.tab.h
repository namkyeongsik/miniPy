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
     NUMBER = 258,
     IDENT = 259,
     PRINT_KW = 260,
     INPUT_KW = 261,
     IF_KW = 262,
     ELSE_KW = 263,
     WHILE_KW = 264,
     FOR_KW = 265,
     IN_KW = 266,
     RANGE_KW = 267,
     RETURN_KW = 268,
     BREAK_KW = 269,
     CONTINUE_KW = 270,
     DEF_KW = 271,
     NEWLINE = 272,
     INDENT = 273,
     DEDENT = 274,
     LE = 275,
     GE = 276,
     EQ = 277,
     NE = 278,
     AND = 279,
     OR = 280,
     UMINUS = 281
   };
#endif
/* Tokens.  */
#define NUMBER 258
#define IDENT 259
#define PRINT_KW 260
#define INPUT_KW 261
#define IF_KW 262
#define ELSE_KW 263
#define WHILE_KW 264
#define FOR_KW 265
#define IN_KW 266
#define RANGE_KW 267
#define RETURN_KW 268
#define BREAK_KW 269
#define CONTINUE_KW 270
#define DEF_KW 271
#define NEWLINE 272
#define INDENT 273
#define DEDENT 274
#define LE 275
#define GE 276
#define EQ 277
#define NE 278
#define AND 279
#define OR 280
#define UMINUS 281




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 12 "parser/parser.y"
{
    int   ival;
    char* sval;
    AST*  node;
}
/* Line 1529 of yacc.c.  */
#line 107 "build/parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

