#include <stdio.h>
#include <stdlib.h>
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
void add_history(char* unsuded) {}

#else // using a unix based machine
#include <editline/readline.h>
#include <editline/history.h>
#endif
// Declare a buffer or user input
// static char input[2048];
//using rediline history now to store input dynamicly

long eval_op(long x, char* op, long y) {
  if (!strcmp(op, "+")) { return x + y; }
  if (!strcmp(op, "-")) { return x - y; }
  if (!strcmp(op, "*")) { return x * y; }
  if (!strcmp(op, "/")) { return x / y; }
  if (!strcmp(op, "%")) { return x % y; }
  if (!strcmp(op, "^")) {
    long product = 1;
    for (int i = 0; i < y; i++) {
      product *= x;
      }
     return product;
   }
  if (!strcmp(op, "min")) { return x < y ? x : y; }
  if (!strcmp(op, "max")) { return x > y ? x : y; }
  return 0;
}

long eval (mpc_ast_t* t) {
  // base case
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }
  // operator is always the secound child
  char* op = t->children[1]->contents;
  //printf("%d\n",t->children_num); help me find the number of children to filter with
  if (!strcmp(op, "-") && t->children_num == 4)
    { return -1 * eval(t->children[2]); }

  // evaluate expr arg
  long x = eval(t->children[2]);
  // iterate the remaining children and combine
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}

long numberOfNodes(mpc_ast_t* t) {
  if (t->children_num == 0) { return 1; }
  if (t->children_num >= 1) {
    int total = 1; // counts the node were looking at
    for (int i = 0; i < t->children_num; i++) {
      total = total + numberOfNodes(t->children[i]);
    }
    return total;
  }

  long numberOfBranches(mpc_ast_t* t){
    return numberOfNodes(t) - 1;
  }

  long numberOfChildren(mpc_ast_t* t) {
    return t->children_num;
  }

  return 0;
}

int main(int argc, char** argv){
  // create parsers
   mpc_parser_t* Number   = mpc_new("number");
   mpc_parser_t* Operator = mpc_new("operator");
   mpc_parser_t* Expr     = mpc_new("expr");
   mpc_parser_t* Lispish  = mpc_new("lispish");

  //define them with the following language
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/;                              \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ; \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispish    : /^/ <operator> <expr>+ /$/ ;           \
    ",
    Number, Operator, Expr, Lispish);

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
        long result = eval(r.output);
        printf("%li\n", result);
        mpc_ast_delete(r.output);
      } else {
        /* Otherwise print and delete the Error */
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
      }
    // free the input b/c its stored on heap
    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispish);

  return 0;
}
