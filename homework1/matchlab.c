/**
 *
 * homework1.c
 * u0669715
 * 
 * Main file for homework1.
 */
#include <stdio.h>
#include "tests.c"


int is_numeric(char a){
  if(a < 48 || a > 57)
    return 0;

  return 1;
}

/**
 * Checks to see if it matches
 *
 * k % 2 == 0, where k is number of 'e'
 * Between 3 and 4 'z'
 * Between 1 and 3 decimal digits
 *
 * Returns 1 for a match or something else if it doesn't
 */
int match_a(char* string){
  int e_even = 1;
  int total_z = 0;
  int total_digits = 0;

  int mode = 0; // 0 = e, 1 = z, 2 = digits
  int i = 0;
  while(string[i] != '\0'){
    char c = string[i];

    switch(mode){
    case 0:
      if(c == 'e')
	e_even = e_even ^ 1;
      else if(c == 'z'){
	mode++;
	i--;
      }
      else return 0;
      break;
    case 1:
      if(c == 'z')
	total_z++;
      else if(is_numeric(c)){
	mode++;
	i--;
      }
      else return 0;
      break;
    case 2:
      if(is_numeric(c)) 
	total_digits++;
      else return 0;
      break;
    }

    i++;
  }

  if(!e_even)
    return 0;
  if(total_z < 3 || total_z > 4)
    return 0;
  if(total_digits < 1 || total_digits > 3)
    return 0;

  return 1;
}

int match_b(char* string){

  return 0; // TODO
}

int match_c(char* string){

  return 0; // TODO
}

int main(int argc, char* argv[]){
  char match_type = 'a'; // match a, b, or c
  int t_enabled = 0; // -t disabled by default
  int index; // starting index of strings to match
  for(index = 1; index < argc; index++){
    char* argument = argv[index];
    
    if(argument[0] != '-')
      break;

    if(argument[1] == 't')
      t_enabled = 1;
    else
      match_type = argument[1];
  }

  for(; index < argc; index++){
    char* pattern = argv[index];
    int match = 0;

    switch(match_type){
    case 'a':
      match = match_a(pattern);
      break;
    case 'b':
      match = match_b(pattern);
      break;
    case 'c':
      match = match_c(pattern);
      break;
    default:
      printf("That match type does not exist.\n");
      return 1;
    }

    if(match){
      printf("yes\n");
    } else if(!t_enabled) {
      printf("no\n");
    }
  }


  // Uncomment this to run some tests
  run_tests();
  return 0;
}
