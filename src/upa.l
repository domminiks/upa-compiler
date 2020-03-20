/*
Gabriel Lopes dos Santos
Lucas Gutierrez Villas Boas
Paulo Eduardo Paes Salomon
Pedro Felipe Dominguite

Based on: Flex & Bison, published by O'Reilly.
*/

%option noyywrap nodefault yylineno
%{
# include "upa.h"
# include "upa.tab.h"
FILE *f;
%}

EXP	([Ee][-+]?[0-9]+)

%%

"+" {   fprintf(f, "+\tOperador soma\t\t\t%s\n", yytext); return yytext[0];}
"-" {   fprintf(f, "-\tOperador subtracao\t\t%s\n", yytext); return yytext[0];}
"*" {   fprintf(f, "*\tOperador multiplicacao\t\t%s\n", yytext); return yytext[0];}
"/" {   fprintf(f, "/\tOperador divisao\t\t%s\n", yytext); return yytext[0];}
"=" {   fprintf(f, "=\tOperador de distribuicao\t%s\n", yytext); return yytext[0];}
"|" {   return yytext[0];}
"," {   fprintf(f, ",\tOperador de separacao\t\t%s\n", yytext); return yytext[0];}
";" {   fprintf(f, ";\tOperador fim de linha\t\t%s\n", yytext); return yytext[0];}
"(" {   fprintf(f, "(\tInicio Parenteses\t\t%s\n", yytext); return yytext[0];}
"{" {   fprintf(f, "{\tInicio chaves\t\t\t%s\n", yytext); return yytext[0];}
"}" {   fprintf(f, "}\tFinal chaves\t\t\t%s\n", yytext); return yytext[0];} 
")" {   fprintf(f, ")\tFinal parenteses\t\t%s\n", yytext); return yytext[0];}
"%" {   return yytext[0]; }

 /* comparison ops */
">"     { yylval.fn = 1;    fprintf(f, "CMP\tComparador Maior\t\t%s\n", yytext); return CMP; }
"<"     { yylval.fn = 2;    fprintf(f, "CMP\tComparador Menor\t\t%s\n", yytext); return CMP; }
"!="    { yylval.fn = 3;    fprintf(f, "CMP\tComparador Diferente\t\t%s\n", yytext); return CMP; }
"=="    { yylval.fn = 4;    fprintf(f, "CMP\tComparador Igual\t\t%s\n", yytext); return CMP; }
">="    { yylval.fn = 5;    fprintf(f, "CMP\tComparador Maior igual\t%s\n", yytext); return CMP; }
"<="    { yylval.fn = 6;    fprintf(f, "CMP\tComparador Menor igual\t%s\n", yytext); return CMP; }
"E"     { yylval.fn = 7;    fprintf(f, "CMP\tOperador logico E\t\t%s\n", yytext); return CMP; }
"OU"    { yylval.fn = 8;    fprintf(f, "CMP\tOperador logico OU\t\t%s\n", yytext); return CMP; }

 /* keywords */
"se"        {   fprintf(f, "IF\tEstrutura de condicao\t\t%s\n", yytext);  return IF; }
"senao"     {   fprintf(f, "ELSE\tEstrutura de decisao\t\t%s\n", yytext); return ELSE; }
"enquanto"  {   fprintf(f, "WHILE\tEstrutura de decisao\t\t%s\n", yytext); return WHILE; }
"funcao"    {   fprintf(f, "FUNCAO\tModulo de tarefa especifica\t%s\n", yytext); return FUNCAO;  }
"principal" {   fprintf(f, "PRINCIPAL\tModulo principal\t%s\n", yytext);return PRINCIPAL; }
"para"      {   fprintf(f, "FOR\tEstrutura de repeticao for\t%s\n", yytext);return FOR; }
"resto"     {   fprintf(f, "%\tOperador matematico resto\t%s\n", yytext);return RESTO; }
"leia"      {   fprintf(f, "%\tFuncao leia da entrada padrao\t%s\n", yytext); return LEIA; }

 /* built in functions */
"escreva" { yylval.fn = B_print; fprintf(f, "FUNC\tFuncao escreva\t\t\t%s\n", yytext);return FUNC; }
 /* debug hack */
"debug"[0-9]+ { debug = atoi(&yytext[5]); printf("debug set to %d\n", debug); }

 /* names */
[a-zA-Z][a-zA-Z0-9]*  { yylval.s = lookup(yytext);  fprintf(f, "NAME\tNome de variavel ou funcao\t%s\n", yytext); return NAME; }
["][^"]*["] { yylval.st = yytext; fprintf(f, "STRING\tSequencia de caracteres\t\t%s\n", yytext); return STRING;}
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval.d = atof(yytext); fprintf(f, "NUMBER\tNumero\t\t\t\t%s\n", yytext); return NUMBER; }
"//".*
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/] { } 
[ \t]   /* ignore white space */ 
\\\n    /* ignore line continuation */
"\n"    /* ignore new line */
.	{   /* unknown character */ }
%%

int main(int argc, char **argv) {
    f = fopen("./upa-tokens.txt","w+");
    fprintf(f, "Token\tDescricao\t\t\tValor\n");
    if(f == NULL) {
      printf("Error!");   
      exit(1);             
    }
    if(argc > 1) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return (1);
        }
    }
    yyparse();
    fclose(f);
}