/**
 * tests.c
 *
 * A bunch of tests to make creating the command line utility a bit easier.
 */

int match_a(char*);

int test_a(){
  int failures = 0;
  if(match_a("hello")) failures++;
  if(match_a("ezzz123")) failures++;
  if(match_a("eezzzzz908")) failures++;
  if(match_a("eezzz")) failures++;
  if(match_a("eeeeeeeeeeeezzzz7654")) failures++;
  
  return failures;
}




/**
 * Returns the number of tests that failed.
 */
int run_tests(){
  int a_fails = test_a();

  printf("-a failures: %d\n", a_fails);
  printf("TOTAL: %d\n", a_fails);
  return 0;
}
