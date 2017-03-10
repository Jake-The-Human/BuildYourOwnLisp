#include <stdio.h>
// Declare a buffer or user input
static char input[2048];

int main(int argc, char** argv){

  /*Prints the version and how to exit*/
  puts("Lispish Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  for(;;){
    // prompts user
    fputs("lispish> ", stdout);
    // get input from user
    fgets(input, 2048, stdin);
    // prints what you said back to you
    printf("No you're a %s", input);
  }
  return 0;
}
