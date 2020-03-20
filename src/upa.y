/*
Gabriel Lopes dos Santos
Lucas Gutierrez Villas Boas
Paulo Eduardo Paes Salomon
Pedro Felipe Dominguite

Based on: Flex & Bison, published by O'Reilly.
*/

%{
#  include <stdio.h>
#  include <stdlib.h>
#  include "upa.h"
%}

%union{
  struct ast *a;
  double d;
  struct symbol *s;
  struct symlist *sl;
  int fn;
  char *st;
}

//tokens
%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token <st> STRING

%token IF ELSE WHILE DO FUNCAO PRINCIPAL FOR RESTO LEIA

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS

%type <a> exp stmt list explist function
%type <sl> symlist

%start principal

%%

stmt: IF '(' exp ')' '{' list '}'                  { $$ = newflow('I', $3, $6, NULL); }
   | IF '(' exp ')' '{' list '}' ELSE '{' list '}' { $$ = newflow('I', $3, $6, $10);  }
   | WHILE '(' exp ')' '{' list '}'                { $$ = newflow('W', $3, $6, NULL); }
   | FOR '(' exp ';' exp ';' exp ')' '{' list '}'  { $$ = newflowfor('Y', $3, $5, $7, $10); }
;

list: /* nothing */ { $$ = NULL; }
   | stmt list { if ($2 == NULL)
	                $$ = $1;
                      else
			$$ = newast('L', $1, $2); }
   | exp ';' list { if ($3 == NULL)
	                $$ = $1;
                      else
			$$ = newast('L', $1, $3); }
   ;

exp: exp CMP exp          { $$ = newcmp($2, $1, $3); }
   | exp '+' exp          { $$ = newast('+', $1,$3); }
   | exp '-' exp          { $$ = newast('-', $1,$3); }
   | exp '*' exp          { $$ = newast('*', $1,$3); }
   | exp '/' exp          { $$ = newast('/', $1,$3); }
   | exp RESTO exp          { $$ = newast('%', $1,$3); }
   | '|' exp              { $$ = newast('|', $2, NULL); }
   | '(' exp ')'          { $$ = $2; }
   | '-' exp %prec UMINUS { $$ = newast('M', $2, NULL); }
   | NUMBER               { $$ = newnum($1); }
   | FUNC '(' STRING  ')' { $$ = newstring($3); }
   | FUNC '(' explist ')' { $$ = newfunc($1, $3); }
   | NAME                 { $$ = newref($1); }
   | NAME '=' exp         { $$ = newasgn($1, $3); }
   | NAME '(' explist ')' { $$ = newcall($1, $3); }
   | LEIA '(' NAME ')'    { $$ = newasgnname($3); }
;

explist: exp
 | exp ',' explist  { $$ = newast('L', $1, $3); }
;
symlist: NAME       { $$ = newsymlist($1, NULL); }
 | NAME ',' symlist { $$ = newsymlist($1, $3); }
;

function: 
| function FUNCAO NAME '(' symlist ')' '{' list '}' {
                       dodef($3, $5, $8);
                       }
 ;

principal: 
| function PRINCIPAL '(' ')' '{' list '}' {
   if($1 == NULL) {
      if(debug) dumpast($6, 0);
      eval($6);
      treefree($6);
   } else {
     if(debug) dumpast($6, 0);
     eval($6);
     treefree($6); 
   }
}
  ;
%%