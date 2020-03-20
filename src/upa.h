/*
Gabriel Lopes dos Santos
Lucas Gutierrez Villas Boas
Paulo Eduardo Paes Salomon
Pedro Felipe Dominguite

Based on: Flex & Bison, published by O'Reilly.
*/

//symbol table
struct symbol {		
  char *name; //variable name
  double value;
  struct ast *func;	//statements for the function
  struct symlist *syms; //list of arguments in declaration
};

//symbol table
#define NHASH 9997
struct symbol symtab[NHASH];

struct symbol *lookup(char*);

//list of symbols, for the argument list of the function
struct symlist {
  struct symbol *sym;
  struct symlist *next;
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

/* node types
 *  + - * / |
 *  0-8 comparison options (for instance, 7 is logical AND (E), 8 is logical OR (OU))
 *  K declares a number (constant)
 *  S recognizes a string
 *  L statement list
 *  I IF statement (se)
 *  W WHILE statement (enquanto)
 *  Y FOR statement (para)
 *  N symbol ref (variable name)
 *  = assignment
 *  S list of symbols (arguments)
 *  F built in function call (call function 'escreva')
 *  C user function call
 *  H read input from user (leia)
 */ 

enum bifs {	//built-in functions
  B_sqrt = 1,
  B_exp,
  B_log,
  B_print //only the print (escreva) was implemented
};

//functios to create new nodes in the Abstract Syntax Tree

struct ast {
  int nodetype;
  struct ast *l;
  struct ast *r;
};

struct fncall {	//built-in function
  int nodetype;	//type F
  struct ast *l;
  enum bifs functype;
};

struct ufncall { //user function
  int nodetype;	//type C
  struct ast *l; //list of arguments
  struct symbol *s;
};

struct flow {
  int nodetype;	//type I (se) or W (enquanto)
  struct ast *cond;	//condition
  struct ast *tl;	//not used
  struct ast *el;	//optional else (senao) list
};

struct flowfor {
  int nodetype;	//type Y
  struct ast *control;	//control variable
  struct ast *cond;	//stop condition
  struct ast *range;	//increment or decrement control variable
  struct ast *tl; //list of actions 
};

struct numval {
  int nodetype;	//type K
  double number;
};

struct string {
  int nodetype;	//type S
  char *s;
};

struct symref {
  int nodetype;	//type N
  struct symbol *s;
};

struct symasgn {
  int nodetype;	//type =
  struct symbol *s;
  struct ast *v;	//value to be assigned
};

//prototype definitions
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newasgnname(struct symbol *s);
struct ast *newnum(double d);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newflowfor(int nodetype, struct ast *ctrl, struct ast *cond, struct ast *range, struct ast *todo);
struct ast *newstring(char *s);

//to define a function
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

//parse each node of the tree, to perform the action (evaluate the AST)
double eval(struct ast *);

//delete and free the AST
void treefree(struct ast *);

//to show in which line a error was found
extern int yylineno;
void yyerror(char *s, ...);

//for debugging
extern int debug;
void dumpast(struct ast *a, int level);