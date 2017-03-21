#include "mpc.h"

#ifdef _WIN32 // this macro is define if this code is running on a win 32 or 64
#include <string.h>
static char* buffer[2048]
// fake readline fuction
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  // make a copy on the heap to emulate
  // readline func also nedd to make room
  // for null byte
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

// fake add_history
// windows does this by defualt
void add_history(char* unused) {}

#else // using a unix based machine
#include <editline/readline.h>
#include <editline/history.h>
#endif
// Declare a buffer or user input
// static char input[2048];
//using rediline history now to store input dynamicly
// create enum of lval types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

typedef struct lval{
  int type;
  long num;
  // store errors and sym as strings
  char* err;
  char* sym;
  //count and pointer to list of lval pointers
  int count;
  struct lval** cell;
} lval;

// lval constructor
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}
// lval_err constructor
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}
// lval_sym constructor
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}
// lcal_sexpre constructor
lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}
// lval destructor
void lval_del(lval* v) {

  switch (v->type) {
    // do nothing for num type
    case LVAL_NUM:
      break;
    // for err or sym we need to free the strings
    case LVAL_ERR:
      free(v->err);
      break;
    case LVAL_SYM:
      free(v->sym);
      break;
    // if sexpr then delete all elements
    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      //free cell
      free(v->cell);
      break;
  }
  //free struct its self
  free(v);
}

lval* lval_add(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

// extracts a single element from the an S-Expression at index i
// and shifts the rest of the list backward so that it no longer contains that
// lval
lval* lval_pop(lval* v, int i) {
  lval* x = v->cell[i];
  // sift memory afer the item at i over the top
  memmove(&v->cell[i], &v->cell[i+1],
    sizeof(lval*) * (v->count-i-1));
    //decrease the count of items in the list
    v->count--;
    //reallocate the mem unsuded
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}
// extracts a single element from the an S-Expression at index i
// and deletes the list it has extracted the element from
lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    // print value contain within
    lval_print(v->cell[i]);
    //dont print trailing space if last elements
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

// prints an lval
void lval_print(lval* v) {
  switch (v->type) {
    case LVAL_NUM:
      printf("%li", v->num);
      break;
    case LVAL_ERR:
      // print what type of error it is
      printf("Error: %s", v->err);
      break;
    case LVAL_SYM:
      //print sym
      printf("%s", v->sym);
      break;
    case LVAL_SEXPR:
      lval_expr_print(v, '(', ')');
      break;
  }
}

// prints value with newline
void lval_printnl(lval* v) { lval_print(v); putchar('\n');}

lval* builtin_op(lval* a, char* op) {

  // makes sure all args are nums
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("can not operate of a non num!");
    }
  }
  //pop the first element
  lval* x = lval_pop(a, 0);
  //f no arg and sub then perform a unary negation
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }
  // while there are still elements remaining
  while (a->count > 0) {
    //pop the next elements
    lval* y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        x = lval_err("Division By Zero!");
        break;
      }
      x->num /= y->num;
     }
    if (strcmp(op, "%") == 0) {
     if (y->num == 0) {
       lval_del(x);
       lval_del(y);
       x = lval_err("Division By Mod!");
       break;
     }
     x->num %= y->num;
    }

    lval_del(y);
  }
  lval_del(a);
  return x;
}

lval* lval_eval(lval* v);

lval* lval_eval_sexpr(lval* v) {
  //evaluate children
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(v->cell[i]);
  }
  // error checkinng
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }
  //empty expression
  if (v->count == 0) { return v; }
  //single expression
  if (v->count == 1) { return lval_take(v, 0); }

  //ensure teh first element is symbol
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f);
    lval_del(v);
    return lval_err("S-expression does not start with a symbal");
  }
  //can builtin with operator
  lval* result = builtin_op(v, f->sym);
  lval_del(f);
  return result;
}

lval* lval_eval(lval* v) {
  //evaluate sexpr
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
// all other lval types remain the same
  return v;
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
  lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
  //if symbol or number conversion to that type
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  // if root (>) or sexpr then create empty list
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strcmp(t->tag, "sexpr"))  { x = lval_sexpr(); }

  // fill this list with any valid expression
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }
  return x;
}

int main(int argc, char** argv){
  // create parsers
   mpc_parser_t* Number   = mpc_new("number");
   mpc_parser_t* Symbol   = mpc_new("symbol");
   mpc_parser_t* Sexpr    = mpc_new("sexpr");
   mpc_parser_t* Expr     = mpc_new("expr");
   mpc_parser_t* Lispish  = mpc_new("lispish");

  //define them with the following language
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                          \
      number : /-?[0-9]+/ ;                    \
      symbol : '+' | '-' | '*' | '/' | '%';         \
      sexpr  : '(' <expr>* ')' ;               \
      expr   : <number> | <symbol> | <sexpr> ; \
      lispish  : /^/ <expr>* /$/ ;             \
    ",
    Number, Symbol, Sexpr, Expr, Lispish);

  /*Prints the version and how to exit*/
  puts("Lispish Version 0.0.0.4.2");
  puts("Made in the USA, By Jake Adamson");
  puts("Press Ctrl+c or :q to Exit\n");

  for(;;){

    char* input = readline("lispish> ");
    add_history(input);

    if(!strcmp(input, ":q")) {
        free(input);
        break;
      }

    /* Attempt to parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispish, &r)) {
        /* On success print and delete the AST */
        //mpc_ast_print(r.output);
        // printf("%d\n", r)
        lval* x = lval_eval(lval_read(r.output));
        lval_printnl(x);
        lval_del(x);
        mpc_ast_delete(r.output);
      } else {
        /* Otherwise print and delete the Error */
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
      }
    // free the input b/c its stored on heap
    free(input);
  }

  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispish);

  return 0;
}
