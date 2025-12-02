%option noyywrap
%option noinput

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

/* 들여쓰기 스택 */
static int indent_stack[256] = {0};
static int indent_top = 0; /* 현재 인덴트 값 */
static int pending_dedents = 0;

#define YY_DECL int yylex1(void)
int yylex(void);
static int lex_debug = -1;

static int calc_indent(const char* text, int len) {
    int n = 0;
    for (int i = 0; i < len; ++i) {
        if (text[i] == '\t') n += 4;
        else if (text[i] == ' ') n += 1;
        else break;
    }
    return n;
}
%}

%x INDENTSTATE

%%

<INITIAL>[ \t]+                 { /* 중간 공백 무시 */ }

<INITIAL>\n                     { BEGIN(INDENTSTATE); return NEWLINE; }

<INITIAL>"#".*                  { /* 주석 */ }

<INITIAL>"print"                { return PRINT_KW; }
<INITIAL>"input"                { return INPUT_KW; }
<INITIAL>"if"                   { return IF_KW; }
<INITIAL>"else"                 { return ELSE_KW; }
<INITIAL>"while"                { return WHILE_KW; }
<INITIAL>"for"                  { return FOR_KW; }
<INITIAL>"in"                   { return IN_KW; }
<INITIAL>"range"                { return RANGE_KW; }
<INITIAL>"def"                  { return DEF_KW; }
<INITIAL>"return"               { return RETURN_KW; }
<INITIAL>"break"                { return BREAK_KW; }
<INITIAL>"continue"             { return CONTINUE_KW; }

<INITIAL>"<="                   { return LE; }
<INITIAL>">="                   { return GE; }
<INITIAL>"=="                   { return EQ; }
<INITIAL>"!="                   { return NE; }
<INITIAL>"&&"                   { return AND; }
<INITIAL>"||"                   { return OR; }

<INITIAL>[0-9]+                 { yylval.ival = atoi(yytext); return NUMBER; }
<INITIAL>[a-zA-Z_][a-zA-Z0-9_]* {
                                  yylval.sval = strdup(yytext);
                                  return IDENT;
                              }

<INITIAL>":"                    { return ':'; }
<INITIAL>"("                    { return '('; }
<INITIAL>")"                    { return ')'; }
<INITIAL>"+"                    { return '+'; }
<INITIAL>"-"                    { return '-'; }
<INITIAL>"*"                    { return '*'; }
<INITIAL>"/"                    { return '/'; }
<INITIAL>"%"                    { return '%'; }
<INITIAL>"<"                    { return '<'; }
<INITIAL>">"                    { return '>'; }
<INITIAL>"="                    { return '='; }
<INITIAL>"!"                    { return '!'; }
<INITIAL>"&"                    { return '&'; }

<INITIAL>.                      { return yytext[0]; }

<INDENTSTATE>^[ \t]*\n          { /* 빈 줄: 무시하고 다음 줄 인덴트 검사 계속 */ }
<INDENTSTATE>^[ \t]*#.*\n       { /* 공백+주석만 있는 줄: 무시 */ }
<INDENTSTATE>[ \t]*[^ \t\n]     {
                                  int n = calc_indent(yytext, yyleng - 1); /* 마지막 한 글자는 비공백 */
                                  int current = indent_stack[indent_top];
                                  char c = yytext[yyleng - 1];
                                  if (n > current) {
                                      indent_stack[++indent_top] = n;
                                      BEGIN(INITIAL);
                                      unput(c);
                                      return INDENT;
                                  } else if (n < current) {
                                      while (indent_top > 0 && n < indent_stack[indent_top]) {
                                          pending_dedents++;
                                          indent_top--;
                                      }
                                      BEGIN(INITIAL);
                                      unput(c);
                                      if (pending_dedents > 0) {
                                          pending_dedents--;
                                          return DEDENT;
                                      }
                                  } else {
                                      BEGIN(INITIAL);
                                      unput(c);
                                  }
                                  /* 인덴트 토큰 없으면 계속 진행 */
                              }

<<EOF>>                        {
                                  if (pending_dedents > 0) {
                                      pending_dedents--;
                                      return DEDENT;
                                  }
                                  if (indent_top > 0) {
                                      indent_top--;
                                      return DEDENT;
                                  }
                                  return 0;
                              }

%%

int yylex(void) {
    if (lex_debug < 0) {
        lex_debug = (getenv("LEXDEBUG") != NULL);
    }
    if (pending_dedents > 0) {
        pending_dedents--;
        return DEDENT;
    }
    int tok = yylex1();
    if (lex_debug) {
        fprintf(stderr, "TOK %d: %s\n", tok, yytext);
    }
    return tok;
}
