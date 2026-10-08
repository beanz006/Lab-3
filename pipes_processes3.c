#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char **argv)
{
  int pipefd[2];
  int pipefd2[2];
  int pid;
  int pid2;

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  if (argc != 2){
    printf("Usage: %s <search_term>\n", argv[0]);
    return 1;
  }

  if(pipe(pipefd) == -1 || pipe(pipefd2) == -1){
    perror("pipe");
    return 1;
  }

  // make a pipe (fds go in pipefd[0] and pipefd[1])

  pid = fork();
  
  if (pid < 0){
    perror("fork");
    return 1;
  }

  if (pid == 0){
    pid2 = fork();

    if(pid2 < 0){
      perror("fork");
      return 1;
    }
    if(pid2 == 0){
      dup2(pipefd2[0], 0);
      
      close(pipefd[0]);
      close(pipefd[1]);
      close(pipefd2[0]);
      close(pipefd2[1]);

      execvp("sort", sort_args);
      perror("execvp sort");
      exit(1);
    }
    else {
      dup2(pipefd[0],0);
      dup2(pipefd2[1],1);

      close(pipefd[0]);
      close(pipefd[1]);
      close(pipefd2[0]);
      close(pipefd2[1]);

      execvp("grep", grep_args);
      perror("execvp grep");
      exit(1);
    }
  }
  else
    {
      // parent gets here and handles "cat scores"

      // replace standard output with output part of pipe

      dup2(pipefd[1], 1);

      // close unused unput half of pipe

      close(pipefd[0]);
      close(pipefd[1]);
      close(pipefd2[0]);
      close(pipefd2[1]);

      // execute cat

      execvp("cat", cat_args);
      perror("execvp cat");
      exit(1);
    }
    return 0;
}