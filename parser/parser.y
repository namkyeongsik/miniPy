%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "interp.h"

extern int yylex(void);
void yyerror(const char* s);
%}

%union {
    int   ival;
    char* sval;
    AST*  node;
}

%token <ival> NUMBER
%token <sval> IDENT
%token PRINT_KW INPUT_KW
%token IF_KW ELSE_KW
%token WHILE_KW
%token FOR_KW IN_KW RANGE_KW
%token RETURN_KW BREAK_KW CONTINUE_KW
%token DEF_KW
%token NEWLINE INDENT DEDENT
%token LE GE EQ NE AND OR

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS
%right '!' '&'

%type <node> program stmt_list stmt simple_stmt compound_stmt suite expr opt_expr param_list arg_list func_def

%%

program
    : stmt_list
      {
          interpret($1);
          ast_free($1);
      }
    ;

stmt_list
    : stmt
      { $$ = $1; }
    | stmt_list stmt
      { $$ = ast_stmt_list($1, $2); }
    | stmt_list func_def
      { $$ = ast_stmt_list($1, $2); }
    | func_def
      { $$ = $1; }
    ;

stmt
    : simple_stmt NEWLINE
      { $$ = $1; }
    | compound_stmt
      { $$ = $1; }
    | NEWLINE
      { $$ = NULL; }
    ;

simple_stmt
    : IDENT '=' expr
      { $$ = ast_assign(ast_var($1), $3); }
    | '*' expr '=' expr
      { $$ = ast_assign(ast_deref($2), $4); }
    | IDENT '(' arg_list ')'
      { $$ = ast_call($1, $3); }
    | IDENT '(' ')' 
      { $$ = ast_call($1, NULL); }
    | PRINT_KW '(' expr ')'
      { $$ = ast_printf($3); }
    | IDENT '=' INPUT_KW '(' ')'
      { $$ = ast_scanf($1); }
    | BREAK_KW
      { $$ = ast_break(); }
    | CONTINUE_KW
      { $$ = ast_continue(); }
    | RETURN_KW opt_expr
      { $$ = ast_return($2); }
    | expr
      { $$ = $1; }
    ;

compound_stmt
    : IF_KW expr ':' suite
      { $$ = ast_if($2, $4, NULL); }
    | IF_KW expr ':' suite ELSE_KW ':' suite
      { $$ = ast_if($2, $4, $7); }
    | WHILE_KW expr ':' suite
      { $$ = ast_while($2, $4, NULL); }
    | FOR_KW IDENT IN_KW RANGE_KW '(' expr ',' expr ')' ':' suite
      {
          char* v1 = strdup($2);
          char* v2 = strdup($2);
          char* v3 = strdup($2);
          char* v4 = strdup($2);
          AST* init = ast_assign(ast_var(v1), $6);
          AST* cond = ast_bin(AST_LT, ast_var(v2), $8);
          AST* step = ast_assign(ast_var(v3), ast_bin(AST_ADD, ast_var(v4), ast_int(1)));
          AST* loop = ast_while(cond, $11, step);
          $$ = ast_stmt_list(init, loop);
      }
    | FOR_KW IDENT IN_KW RANGE_KW '(' expr ',' expr ',' expr ')' ':' suite
      {
          char* v1 = strdup($2);
          char* v2 = strdup($2);
          char* v3 = strdup($2);
          char* v4 = strdup($2);
          AST* init = ast_assign(ast_var(v1), $6);
          AST* cond = ast_bin(AST_LT, ast_var(v2), $8);
          AST* step = ast_assign(ast_var(v3), ast_bin(AST_ADD, ast_var(v4), $10));
          AST* loop = ast_while(cond, $13, step);
          $$ = ast_stmt_list(init, loop);
      }
    ;

suite
    : simple_stmt NEWLINE
      { $$ = $1; }
    | NEWLINE INDENT stmt_list DEDENT
      { $$ = ast_block($3); }
    ;

expr
    : NUMBER
      { $$ = ast_int($1); }
    | IDENT
      { $$ = ast_var($1); }
    | IDENT '(' arg_list ')'
      { $$ = ast_call($1, $3); }
    | IDENT '(' ')'
      { $$ = ast_call($1, NULL); }

    | expr '+' expr
      { $$ = ast_bin(AST_ADD, $1, $3); }
    | expr '-' expr
      { $$ = ast_bin(AST_SUB, $1, $3); }
    | expr '*' expr
      { $$ = ast_bin(AST_MUL, $1, $3); }
    | expr '/' expr
      { $$ = ast_bin(AST_DIV, $1, $3); }
    | expr '%' expr
      { $$ = ast_bin(AST_MOD, $1, $3); }

    | expr '<' expr
      { $$ = ast_bin(AST_LT, $1, $3); }
    | expr '>' expr
      { $$ = ast_bin(AST_GT, $1, $3); }
    | expr LE expr
      { $$ = ast_bin(AST_LE, $1, $3); }
    | expr GE expr
      { $$ = ast_bin(AST_GE, $1, $3); }
    | expr EQ expr
      { $$ = ast_bin(AST_EQ, $1, $3); }
    | expr NE expr
      { $$ = ast_bin(AST_NE, $1, $3); }

    | expr AND expr
      { $$ = ast_bin(AST_AND, $1, $3); }
    | expr OR expr
      { $$ = ast_bin(AST_OR, $1, $3); }

    | '!' expr
      { $$ = ast_unary(AST_NOT, $2); }
    | '&' IDENT
      { $$ = ast_addr(ast_var($2)); }
    | '*' expr
      { $$ = ast_deref($2); }

    | '-' expr %prec UMINUS
      { $$ = ast_bin(AST_SUB, ast_int(0), $2); }
    | '(' expr ')'
      { $$ = $2; }
    ;

func_def
    : DEF_KW IDENT '(' param_list ')' ':' suite
      { $$ = ast_func_def($2, $4, $7); }
    | DEF_KW IDENT '(' ')' ':' suite
      { $$ = ast_func_def($2, NULL, $6); }
    ;

param_list
    : IDENT
      { $$ = ast_var($1); }
    | param_list ',' IDENT
      { $$ = ast_stmt_list($1, ast_var($3)); }
    ;

arg_list
    : expr
      { $$ = $1; }
    | arg_list ',' expr
      { $$ = ast_stmt_list($1, $3); }
    ;

opt_expr
    : /* empty */ { $$ = NULL; }
    | expr        { $$ = $1; }
    ;

%%

void yyerror(const char* s)
{
    fprintf(stderr, "parse error: %s\n", s);
}
