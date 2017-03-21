#include <stdlib.h>
int main()
{
  char* args[] = {"/usr/bin/ls", NULL};
  char* environ[] = {NULL};
  execve( args[0], args, environ);

  printf("Nope.");
}
