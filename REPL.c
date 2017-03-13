#include <stdio.h>
#include <stdlib.h> // for free func

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
  cpy[strlen(cpy)-1] = '\n';
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

int main(int argc, char** argv){

  /*Prints the version and how to exit*/
  puts("Lispish Version 0.0.0.0.1");
  puts("Made in the USA, By Jake Adamson");
  puts("Press Ctrl+c or :q to Exit\n");

  for(;;){
    // prompts user
    char* input = readline("lispish> ");
    // add input to history
    add_history(input);
    if(input[0] == ':' && input[1] == 'q') {
        free(input);
        break;
      }
    // prints what you said back to you
    printf("%ssssssss\n", input);

    // free the input b/c its stored on heap
    free(input);
  }
  return 0;
}
