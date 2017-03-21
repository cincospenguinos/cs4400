#include <unistd.h>
#include <stdio.h>

// pipe  (int* fds)
//
// fds should be an array of 2
// creates a new open "file"
// with fds[1] writing to that file
// and  fds[0] reading from that file


// dup2 (int old_fd, int new_fd)
// 
// sets new_fd so that it refers to the same
// file as old_fd

// Create two children, one to exec reader, one to exec writer
// Set the standard input of reader 
// to the same "file" as the standard output of writer
int main(int argc, char** argv)
{

  if(argc != 4){
    printf("Three args are required\n");
    return 1;
  }
 
  char* writer_args[] = {argv[1], NULL};
  char* reader_args[] = {argv[2], NULL};
  char* third_args[] = {argv[3], NULL};
  char* env[] = {NULL};

  int fd[2];
  pipe(fd);
  pipe(fd);
  // set up the writer process
  if (fork() == 0) 
  {
    // redirect stdout to the writing end of the pipe
    dup2(fd[1], 1);
    close(fd[0]);
    execve(writer_args[0], writer_args, env);
  }

  // set up the reader process
  if (fork() == 0) 
  {    
    // redirect stdin to the reading end of the pipe
    dup2(fd[0], 0);
    close(fd[1]);
    execve(reader_args[0], reader_args, env);
  }

  if(fork() == 0){
    dup2(fd[0], 0);
    close(fd[1]);
    execve(third_args[0], third_args, env);
  }
 
  close(fd[0]);
  close(fd[1]);

  // Wait for two children
  int status;
  wait(&status);
  wait(&status);
  wait(&status);
}
