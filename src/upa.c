/*
Gabriel Lopes dos Santos
Lucas Gutierrez Villas Boas
Paulo Eduardo Paes Salomon
Pedro Felipe Dominguite

Based on: Flex & Bison, published by O'Reilly.
*/

#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <math.h>
#  include "upa.h"

double aux;

//symbol table using hash
static unsigned symhash(char *sym) {
  unsigned int hash = 0;
  unsigned c;

  while(c = *sym++) hash = hash*9 ^ c;

  return hash;
}

struct symbol * lookup(char* sym) {
  struct symbol *sp = &symtab[symhash(sym)%NHASH];
  int scount = NHASH;

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) { return sp; }

    if(!sp->name) {	//new entry
      sp->name = strdup(sym);
      sp->value = 0;
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if(++sp >= symtab+NHASH) sp = symtab; //look for the next available entry
  }
  yyerror("symbol table overflow.\n");
  abort(); //exit if table is full os symbols

}

struct ast * newast(int nodetype, struct ast *l, struct ast *r) {
  struct ast *a = malloc(sizeof(struct ast));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast * newnum(double d) {
  struct numval *a = malloc(sizeof(struct numval));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = 'K';
  a->number = d;
  return (struct ast *)a;
}

struct ast * newcmp(int cmptype, struct ast *l, struct ast *r) {
  struct ast *a = malloc(sizeof(struct ast));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast * newfunc(int functype, struct ast *l) {
  struct fncall *a = malloc(sizeof(struct fncall));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;
  return (struct ast *)a;
}

struct ast * newcall(struct symbol *s, struct ast *l) {
  struct ufncall *a = malloc(sizeof(struct ufncall));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = 'C';
  a->l = l;
  a->s = s;
  return (struct ast *)a;
}

struct ast * newref(struct symbol *s) {
  struct symref *a = malloc(sizeof(struct symref));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
}

struct ast * newasgn(struct symbol *s, struct ast *v) {
  struct symasgn *a = malloc(sizeof(struct symasgn));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

struct ast * newasgnname(struct symbol *s) {
  struct symasgn *a = malloc(sizeof(struct symasgn));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = 'H';
  a->s = s;
  a->v = NULL;
  return (struct ast *)a;
}

struct ast * newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el) {
  struct flow *a = malloc(sizeof(struct flow));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;
  return (struct ast *)a;
}

struct ast * newflowfor(int nodetype, struct ast *ctrl, struct ast *cond, struct ast *range, struct ast *todo) {
  struct flowfor *a = malloc(sizeof(struct flowfor));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->control = ctrl;
  a->range = range;
  a->tl = todo;
  return (struct ast *)a;
}

struct ast * newstring(char* s) {
  struct string *a = malloc(sizeof(struct string));
  
  if(!a) {
    yyerror("out of space.");
    exit(0);
  }
  //Defining the end of the string
  int i = 1;
  while(s[i] != '"') {
    i++;
  }
  a->s = malloc(i*sizeof(char));
  a->nodetype = 'S';
  strncpy(a->s, &s[1], i-1);
  return (struct ast *)a;
}

struct symlist * newsymlist(struct symbol *sym, struct symlist *next) {
  struct symlist *sl = malloc(sizeof(struct symlist));
  
  if(!sl) {
    yyerror("out of space.");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  return sl;
}

void symlistfree(struct symlist *sl) {
  struct symlist *nsl;

  while(sl) {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}

//to define a new function
void dodef(struct symbol *name, struct symlist *syms, struct ast *func) {
  if(name->syms) symlistfree(name->syms);
  if(name->func) treefree(name->func);
  name->syms = syms;
  name->func = func;
}

static double callbuiltin(struct fncall *);
static double calluser(struct ufncall *);

double eval(struct ast *a) {
  double v;

  if(!a) {
    yyerror("internal error, null eval.");
    return -1;
  }

  switch(a->nodetype) {
    //constant or number
    case 'K': v = ((struct numval *)a)->number; break;

    //name reference (variable)
    case 'N': v = ((struct symref *)a)->s->value; break;

    //assignment
    case '=': v = ((struct symasgn *)a)->s->value =
        eval(((struct symasgn *)a)->v); break;

    //read from stdin and assign
    case 'H':
      scanf("%lf", &aux);
      ((struct symasgn *)a)->v = newnum(aux);
      v = ((struct symasgn *)a)->s->value = eval(((struct symasgn *)a)->v); break;

    //math and logical expressions
    case '+': v = eval(a->l) + eval(a->r); break;
    case '-': v = eval(a->l) - eval(a->r); break;
    case '*': v = eval(a->l) * eval(a->r); break;
    case '/': v = eval(a->l) / eval(a->r); break;
    case '%': v = fmod(eval(a->l), eval(a->r)); break;
    case '|': v = fabs(eval(a->l)); break;
    case 'M': v = -eval(a->l); break;

    //comparisons
    case '1': v = (eval(a->l) > eval(a->r))? 1 : 0; break;
    case '2': v = (eval(a->l) < eval(a->r))? 1 : 0; break;
    case '3': v = (eval(a->l) != eval(a->r))? 1 : 0; break;
    case '4': v = (eval(a->l) == eval(a->r))? 1 : 0; break;
    case '5': v = (eval(a->l) >= eval(a->r))? 1 : 0; break;
    case '6': v = (eval(a->l) <= eval(a->r))? 1 : 0; break;
    case '7': v = (eval(a->l) && eval(a->r))? 1 : 0; break;
    case '8': v = (eval(a->l) || eval(a->r))? 1 : 0; break;

    //control flows
    //is/else (se/senao) statements
    case 'I': 
      if( eval( ((struct flow *)a)->cond) != 0) {
        if( ((struct flow *)a)->tl) {
    v = eval( ((struct flow *)a)->tl);
        } else
    v = 0.0;		/* a default value */
      } else {
        if( ((struct flow *)a)->el) {
          v = eval(((struct flow *)a)->el);
        } else
    v = 0.0;		/* a default value */
      }
      break;
    //while loop (enquanto)
    case 'W':
      v = 0.0;		/* a default value */
      
      if( ((struct flow *)a)->tl) {
        while( eval(((struct flow *)a)->cond) != 0)
    v = eval(((struct flow *)a)->tl);
      }
      break;			/* last value is value */
    //for (para) loop
    case 'Y':
      v = 0.0;
      //If there is a to do list inside the for statement
      if( ((struct flowfor *)a)->tl) {
          //Create control variable
          eval(((struct flowfor *)a)->control);
          while( eval(((struct flowfor *)a)->cond) != 0) {
            v = eval(((struct flowfor *)a)->tl);
            eval(((struct flowfor *)a)->range); 
          }
        }
        break;
    //list of statements
    case 'L': eval(a->l); v = eval(a->r); break;
    //call print function (escreva)
    case 'F': v = callbuiltin((struct fncall *)a); break;
    //call user defined function
    case 'C': v = calluser((struct ufncall *)a); break;
    //print string
    case 'S': printf("%s\n", ((struct string *)a)->s); break;

    default: printf("internal error: bad node %c\n", a->nodetype);
  }
  return v;
}

static double callbuiltin(struct fncall *f) {
  enum bifs functype = f->functype;
  double v = eval(f->l);

  switch(functype) {
    case B_sqrt:
      return sqrt(v);
    case B_exp:
      return exp(v);
    case B_log:
      return log(v);
    case B_print:
      printf("%g\n", v);
      return v;
    default:
      yyerror("Unknown built-in function %d", functype);
      return 0.0;
  }
}

static double calluser(struct ufncall *f) {
  struct symbol *fn = f->s; //function name
  struct symlist *sl;	//arguments in the prototype
  struct ast *args = f->l; //arguments provided by the user
  double *oldval, *newval; //save prototype, calculate new values, restore old values
  double v;
  int nargs;
  int i;

  if(!fn->func) {
    yyerror("call to undefined function.", fn->name);
    return 0;
  }

  //count the arguments
  sl = fn->syms;
  for(nargs = 0; sl; sl = sl->next)
    nargs++;

  //create space to save the arguments
  oldval = (double *)malloc(nargs * sizeof(double));
  newval = (double *)malloc(nargs * sizeof(double));

  if(!oldval || !newval) {
    yyerror("Out of space in %s", fn->name); return 0.0;
  }
  
  //check the arguments and call eval for each of them
  for(i = 0; i < nargs; i++) {
    if(!args) {
      yyerror("too few args in call to %s", fn->name);
      free(oldval); free(newval);
      return 0;
    }

    if(args->nodetype == 'L') {	//if this is a list node
      newval[i] = eval(args->l);
      args = args->r;
    } else { //if it's the end of the list
      newval[i] = eval(args);
      args = NULL;
    }
  }
		     
  //save old values and assign new ones
  sl = fn->syms;
  for(i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;
    oldval[i] = s->value;
    s->value = newval[i];
    sl = sl->next;
  }

  free(newval);

  //evaluate the function
  v = eval(fn->func);

  //put the arguments from the prototype back
  sl = fn->syms;
  for(i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;

    s->value = oldval[i];
    sl = sl->next;
  }
  free(oldval);
  return v;
}

//to free the mallocs
void treefree(struct ast *a) {
  switch(a->nodetype) {

  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '1':  case '2':  case '3':  case '4':  case '5':  case '6': case '7': case '8':
  case 'L':
    treefree(a->r);

  case '|':
  case 'M': case 'C': case 'F':
    treefree(a->l);

  //no subtree
  case 'K': case 'N': case 'S':
    break;

  case '=': case 'H':
    free( ((struct symasgn *)a)->v);
    break;

  case 'I': case 'W':
    free( ((struct flow *)a)->cond);
    if( ((struct flow *)a)->tl) free( ((struct flow *)a)->tl);
    if( ((struct flow *)a)->el) free( ((struct flow *)a)->el);
    break;

  case 'Y':
    if( ((struct flowfor *)a)->cond) free( ((struct flowfor *)a)->cond);
    if( ((struct flowfor *)a)->control) free( ((struct flowfor *)a)->control);
    if( ((struct flowfor *)a)->range) free( ((struct flowfor *)a)->range);
    if( ((struct flowfor *)a)->tl) free( ((struct flowfor *)a)->tl);
    break;

  default: printf("internal error: free bad node %c\n", a->nodetype);
  }	  
    free(a);
}

void yyerror(char *s, ...) {
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

//debugging: dump out the AST (never used)
int debug = 0;
void dumpast(struct ast *a, int level) {
  printf("%*s", 2*level, "");
  level++;

  if(!a) {
    printf("NULL\n");
    return;
  }

  switch(a->nodetype) {

  case 'K': printf("number %4.4g\n", ((struct numval *)a)->number); break;

  case 'N': printf("ref %s\n", ((struct symref *)a)->s->name); break;

  case '=': printf("= %s\n", ((struct symref *)a)->s->name);
    dumpast( ((struct symasgn *)a)->v, level); return;

  case '+': case '-': case '*': case '/': case 'L':
  case '1': case '2': case '3':
  case '4': case '5': case '6':
  case '7': case '8':
    printf("binop %c\n", a->nodetype);
    dumpast(a->l, level);
    dumpast(a->r, level);
    return;

  case '|': case 'M': 
    printf("unop %c\n", a->nodetype);
    dumpast(a->l, level);
    return;

  case 'I': case 'W':
    printf("flow %c\n", a->nodetype);
    dumpast( ((struct flow *)a)->cond, level);
    if( ((struct flow *)a)->tl)
      dumpast( ((struct flow *)a)->tl, level);
    if( ((struct flow *)a)->el)
      dumpast( ((struct flow *)a)->el, level);
    return;
	              
  case 'F':
    printf("builtin %d\n", ((struct fncall *)a)->functype);
    dumpast(a->l, level);
    return;

  case 'C': printf("call %s\n", ((struct ufncall *)a)->s->name);
    dumpast(a->l, level);
    return;

  default: printf("bad %c\n", a->nodetype);
    return;
  }
}