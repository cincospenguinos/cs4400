/**
 * matchlab.c
 * u0669715
 * 
 * Main file for homework1.
 */
#include<stdio.h>
#include<string.h>

/* Utility functions */
int is_numeric(char);
int is_upcase(char);
void grow_str(char*, char*);

/* Matching functions */
int match_a(char*);
int match_b(char*);
int match_c(char*);

/* Replace functions */
void replace_b(char*, char*, int);
void replace_c(char*, char*);


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
      if(match_type == 'a'){
	printf("%s\n", pattern);
      } else if(match_type == 'b'){
	int size = (int) strlen(pattern) * 2;
	char buffer[size];
	replace_b(buffer, pattern, size);
	printf("%s\n", buffer);
      } else {
	
      }
    } else if(!t_enabled) {
      printf("no\n");
    }
  }

  return 0;
}


/**
 * Indicates whether or not the char passed is a number or not.
 */
int is_numeric(char a){
  if(a < 48 || a > 57)
    return 0;

  return 1;
}

/**
 * Indicates whether or not the char passed is an uppercase letter or not
 */
int is_upcase(char a){
  if (a < 65 || a > 90)
    return 0;

  return 1;
}

void grow_str(char* tmp, char* source){
  strcpy(tmp, source);
  memcpy(source, tmp, sizeof(tmp));
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
      } else 
	return 0;
      break;
    case 1:
      if(c == 'z')
	total_z++;
      else if(is_numeric(c)){
	mode++;
	i--;
      } else 
	return 0;
      break;
    case 2:
      if(is_numeric(c)) 
	total_digits++;
      else 
	return 0;
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

/**
 * Match a sequence of (with nothing else before or after)
 *
 * any number (including zero) repetitions of the letter “l”;
 * an odd number of uppercase letters — call this sequence X;
 * any odd number of repetitions of the letter “p”;
 * between 1 and 3 (inclusive) decimal digits; and
 * the same characters as the odd-positioned characters in X.
 */
int match_b(char* string){
  int odd_upcase = 0;
  int odd_p = 0;
  int total_digits = 0;
  char capital_letters[10];
  int cap_index = 0;

  int i = 0;
  int mode = 0; // 0 = l, 1 = uppercase, 2 = p, 3 = decimal digits, 4 = extra uppercase letters
  while(string[i] != '\0'){
    char c = string[i];

    switch(mode){
    case 0: // Look for l
      if(c == 'l')
	;
      else if (is_upcase(c)) {
	i--;
	mode++;
      } else return 0;
      break;
    case 1: // Look for uppercase
      if(is_upcase(c))
	odd_upcase = odd_upcase ^ 1;
      else if(c == 'p'){
	i--;
	mode++;
      } else return 0;

      if(i << 31 >= 0) // If this is an even index
	capital_letters[cap_index++] = c;

      if(capital_letters[cap_index] == '\0'){
	int l = (int)strlen(capital_letters) * 2;
	char tmp[l];
	grow_str(tmp, capital_letters);
      }
      break;
    case 2:  // Look for p
      if(c == 'p')
	odd_p = odd_p ^ 1;
      else if(is_numeric(c)){
	i--;
	mode++;
      } else return 0;
      break;
    case 3: // Look for digits
      if(is_numeric(c))
	total_digits++;
      else {
	i--;
	mode++;
      }
      break;
    case 4: // Look for uppercase letters
      while(0){} // This was required to get the compiler to work. I have no clue why.
      int j = 0;
      int flag = 0;
      for(; j < strlen(capital_letters); j++){
	if(c == capital_letters[j]){
	  flag = 1;
	  break;
	}
      }

      if(!flag)
	return 0;

      break;
    }

    i++;
  }

  if(!odd_upcase)
    return 0;
  if(!odd_p)
    return 0;
  if(total_digits < 1 || total_digits > 3)
    return 0;

  return 1; // TODO
}


/**
 * Matches the pattern given the following:
 *
 * odd number of "i"
 * odd number of caps
 * between three and six reps of "t"
 * the caps, reversed
 * between one and three numbers
 */
int match_c(char* string){
  int mode = 0; // 0 = i, 1 = caps, 2 = t, 3 = caps reversed, 4 = numbers
  int i = 0;

  int odd_i = 0;
  char capital_letters[10];
  int cap_index = 0;
  int t_letters = 0;
  char reverse_letters[10];
  int reverse_index = 0;
  int numbers = 0;

  while(string[i] != '\0'){
    char c = string[i];

    switch(mode){
    case 0:
      if(c == 'i')
	odd_i = odd_i ^ 1;
      else if(is_upcase(c)){
	mode++;
	i--;
      } else return 0;
      break;
    case 1:
      if(is_upcase(c))
	capital_letters[cap_index++] = c;
      else if(c == 't'){
	mode++;
	i--;
      } else return 0;

      if(capital_letters[cap_index] == '\0'){
	char tmp[(int)strlen(capital_letters) * 2];
	grow_str(tmp, capital_letters);
      }
      break;
    case 2:
      if(c == 't')
	t_letters++;
      else if(is_upcase(c)){
	mode++;
	i--;
      } else return 0;
      break;
    case 3:
      if(is_upcase(c))
	reverse_letters[reverse_index++] = c;
      else if(is_numeric(c)){
	mode++;
	i--;
      } else return 0;

      if(reverse_letters[reverse_index] == '\0'){
        char tmp[(int)strlen(reverse_letters) * 2];
        grow_str(tmp, reverse_letters);
      }
      
      break;
    case 4:
      if(is_numeric(c))
	numbers++;
      else return 0;
      break;
    }

    i++;
  }

  if(!odd_i)
    return 0;
  if(((int)strlen(capital_letters)) << 31 < 0) // Only odd capital letters
    return 0;
  if(t_letters < 3 || t_letters > 6)
    return 0;
  if(numbers < 1 || numbers > 3)
    return 0;

  // Now let's check out the capital letters
  i = 0;
  int j = (int) strlen(reverse_letters) - 1;

  // First let's make sure that the arrays are setup properly
  while(is_upcase(capital_letters[i])){
    i++;
  }
  capital_letters[i] = '\0';

  i = 0;
  while(is_upcase(reverse_letters[i])){
    i++;
  }
  reverse_letters[i] = '\0';

  // And now let's compare
  i = 0;
  while(j > 0){
    if(capital_letters[i++] != reverse_letters[j--])
      return 0;
  }

  return 1; // TODO
}

void replace_b(char* buffer, char* original, int dest_size){
  int i;
  for(i = 0; i < strlen(original); i++){
    buffer[i * 2] = original[i];
  }

  char num = '0';
  for(i = 1; i < dest_size; i += 2){
    buffer[i] = num++;

    if(num == '8')
      num = '0';
  }

  buffer[dest_size + 1] = '\0';
}

void replace_c(char* buffer, char* original){
  // TODO
}
