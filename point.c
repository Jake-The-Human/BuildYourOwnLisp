#include <stdio.h>

typedef struct{
  int x,y;
} point;

int main(int argc, char** argv){
  point p;
  int i;
  char* str = "hey";
  point* pp = &p;
  p.x = 1000;
  p.y = 90909090;
  printf("%d %d\n", (*pp).x, pp->y);

  for(i = 0; i < 5; i++)
  {
    puts("HELLO DOODER!");
  }
  i = 0;
  while(i < 5)
  {
    puts(str);
    i += 1;
  }
  puts(" ");
  i = 0;
  do{
    puts(str);
    i++;
  } while (i < 5);

  switch (3) {
    case 1:
    puts("1");
    break;
    case 2:
    puts("2");
    break;
    case 3:
    puts("3");
    break;
    default:
    puts("other num");
    break;
  }

  for(i = 0; i < 10;i++)
  {
    if(i % 2 != 0)
      {continue;}
    printf("%d ", i);


  }
  return 0;
}
