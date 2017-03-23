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
static void print_var(script_var *var);

/** Let's take some notes
 *
 * - A "group" is a collection of commands put all together to work together somehow.
 *   Basically just a line with one or more commands on it.
 * - Each group may consist of "AND-COMMANDS" or "OR-COMMANDS". AND means "run each thing
 *   in sequence, piping the output of one into the next one." OR means "run each thing
 *   separately, and finish when one of them is done."
 */

/* CURRENT TODO: Let's now try variables */

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

// TODO: Variables
// TODO: AND Commands
// TODO: OR Commands

static void run_script(script *src) {
  // TODO: You may need to protect yourself here
  int groups = src->num_groups;
  int i;
  for(i = 0; i < groups; i++){
    run_group(&src->groups[i]);
  }
}

static void run_group(script_group *group) {
  int r;
  int i;

  for(r = 0; r < group->repeats; r++) {

    if (group->mode == GROUP_AND) {
      int total_pipes = group->num_commands - 1;
      int pipe_index = 0;
      int fds[total_pipes][2];

      for(i = 0; i < group->num_commands; i++) {
	script_command current_command = group->commands[i];

	int *fd = fds[pipe_index];
        Pipe(fds[pipe_index]);

	if (fork() == 0){
	  if(pipe_index == 0) // First pipe, output to STD_OUT
	    dup2(fd[1], 1);
	  else if(pipe_index - 1 == total_pipes) // Last pipe, input to STD_IN
	    dup2(fd[0], 0);
	  else { // In between, output previous to input next
	    dup2(fd[0], 0);
	    dup2(fd[1], 1);
	  }

	  run_command(&current_command);
	} else {
	  pipe_index++;

	  Close(fd[0]);
	  Close(fd[1]);
	}
      }
    } else if (group->mode == GROUP_OR){

    } else {
      run_command(&group->commands[0]);
    }
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
  Execve(argv[0], (char * const *)argv, environ);
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

static void print_var(script_var *var){
  printf("%s\n", var->name);
  printf("\t%s\n", var->value);
}
