#include<stdio.h>

void greet(char* name){
  printf("Hello %s\n", name);
}

int main(int argc, char* argv[]){
  int i = 1;
  for(; i < argc; i++)
    greet(argv[i]);

  return 0;
}
