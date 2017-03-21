/* This is the main file for the `whoosh` interpreter and the part
   that you modify. */

#include <stdlib.h>
#include <stdio.h>
#include "csapp.h"
#include "ast.h"
#include "fail.h"

static void run_script(script *scr);
static void run_group(script_group *group);
static void run_command(script_command *command);
static void set_var(script_var *var, int new_value);

/** Let's take some notes
 *
 * - A "group" is a collection of commands put all together to work together somehow.
 *   Basically just a line with one or more commands on it.
 * - Each group may consist of "AND-COMMANDS" or "OR-COMMANDS". AND means "run each thing
 *   in sequence, piping the output of one into the next one." OR means "run each thing
 *   separately, and finish when one of them is done."
 */

/* You probably shouldn't change main at all. */

int main(int argc, char **argv) {
  script *scr;
  
  if ((argc != 1) && (argc != 2)) {
    fprintf(stderr, "usage: %s [<script-file>]\n", argv[0]);
    exit(1);
  }

  scr = parse_script_file((argc > 1) ? argv[1] : NULL);

  run_script(scr);

  return 0;
}

static void run_script(script *src) {
  // TODO: You may need to protect yourself here
  int groups = src->num_groups;
  int i;
  for(i = 0; i < groups; i++){
    run_group(&src->groups[i]);
  }
  /*
  if(groups == 1)
    run_group(&src->groups[0]);
  else
  fail("Only 1 group supported at this time");*/
}

static void run_group(script_group *group) {
  /* You'll have to make run_group do better than this, too */
  if (group->repeats != 1)
    fail("only repeat 1 supported");
  if (group->result_to != NULL){
    fail("setting variables not supported");
  }
  if (group->num_commands != 1) {
    fail("only 1 command supported");
  }

  int r;
  for(r = 0; r < group->repeats; r++){
    run_command(&group->commands[0]);
  }
}

/* This run_command function is a good start, but note that it runs
   the command as a replacement for the `whoosh` script, instead of
   creating a new process. */

static void run_command(script_command *command) {
  const char **argv;
  int i;

  argv = malloc(sizeof(char *) * (command->num_arguments + 2));
  argv[0] = command->program;
  
  for (i = 0; i < command->num_arguments; i++) {
    if (command->arguments[i].kind == ARGUMENT_LITERAL)
      argv[i+1] = command->arguments[i].u.literal;
    else
      argv[i+1] = command->arguments[i].u.var->value;
  }
  
  argv[command->num_arguments + 1] = NULL; // argv must end with NULL

  pid_t child = fork();

  if(child == 0){
    Execve(argv[0], (char * const *)argv, environ);
  } else {
    // TODO: I'm pretty sure I want a global of some sort in order to let
    // the parent know what's happening. Maybe I'll return pid_t instead and
    // have run_group() manage all of that.
    int status;
    wait(&status);
    free(argv);
  }
}

/* You'll likely want to use this set_var function for converting a
   numeric value to a string and installing it as a variable's
   value: */

static void set_var(script_var *var, int new_value) {
  char buffer[32];
  free((void *)var->value);
  snprintf(buffer, sizeof(buffer), "%d", new_value);
  var->value = strdup(buffer);
}
