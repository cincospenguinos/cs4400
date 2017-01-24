#include<stdio.h>

void greet(char *name)
{
  printf("%s\n", name);
  name += 2;
  printf("%s\n", name);
}

void something_else(int *i){
  printf("%p\n", i);
  i += 2;
  printf("%p\n", i);
}

int main(int argc, char* argv[])
{
  
  int j = 21;
  int *i = &j;
  something_else(i);
  printf("%i\n", i);


  return 0;
}
