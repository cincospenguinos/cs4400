/* This is the main file for the `whoosh` interpreter and the part
   that you modify. */

#include <stdlib.h>
#include <stdio.h>
#include "csapp.h"
#include "ast.h"
#include "fail.h"

static void run_script(script *scr);
static void run_group(script_group *group);
static void run_command(script_command *command, int *fds);
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

static void run_script(script *src) {
  // TODO: You may need to protect yourself here
  int groups = src->num_groups;
  int i;
  for(i = 0; i < groups; i++){
    run_group(&src->groups[i]);
  }
}

static void run_group(script_group *group) {
  if (group->num_commands != 1) {
    fail("only 1 command supported");
  }

  int fds[2]; // Our file descriptors for this group

  int r;
  for(r = 0; r < group->repeats; r++){
    int i;
    for(i = 0; i < group->num_commands; i++){
      script_command current_command = group->commands[i];
      run_command(&current_command, fds);

      if(group->result_to != NULL){
	char buffer[32];

	// Close our file descriptor
	Close(fds[1]);

	// Wait on the command
	int status;
	wait(&status);

	// Read the input into the buffer
	Read(fds[0], buffer, 31);
	group->result_to->value = buffer;
	print_var(group->result_to);
      }
    }
  }
}

/* This run_command function is a good start, but note that it runs
   the command as a replacement for the `whoosh` script, instead of
   creating a new process. */

static void run_command(script_command *command, int *fds) {
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

  // We need to get the output out from the child process
  Pipe(fds);

  pid_t child = fork();

  if(child == 0){
    dup2(fds[1], 1);
    Execve(argv[0], (char * const *)argv, environ);
  } else {
    // Save the variable if need be
    if(command->pid_to != NULL){
      //printf("Need to set a variable for this command!\n");
      set_var(command->pid_to, child);
    }
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

static void print_var(script_var *var){
  printf("%s\n", var->name);
  printf("\t%s\n", var->value);
}
