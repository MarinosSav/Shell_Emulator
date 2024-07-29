#include "parser/ast.h"
#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


int run_pipe(node_t* node){

    pid_t first_pid, second_pid;
    int fd[2];

    pipe(fd);
    node_t *pipe = node->pipe.parts[0];
    node_t *pipe2 = node->pipe.parts[1];
    first_pid = fork();
    if (first_pid == 0){
      close(fd[0]);
      close(1);
      dup(fd[1]);
      if(strcmp(pipe->command.program, "cd") == 0){
        chdir(pipe->command.argv[1]);
        exit(0);
      }
      else if (strcmp(pipe->command.program, "exit") == 0){
        exit(atoi(pipe->command.argv[1]));

      }
      else{
        if (execvp(pipe->command.program, pipe->command.argv) < 0){
          perror("execvp");
          exit(1);
        }
        exit(0);
      }
    }

    second_pid = fork();
    if (second_pid == 0){
      close(fd[1]);
      close(0);
      dup(fd[0]);
      if(strcmp(pipe2->command.program, "cd") == 0){
        chdir(pipe2->command.argv[1]);
        exit(0);
      }
      else if (strcmp(pipe2->command.program, "exit") == 0){
        exit(atoi(pipe2->command.argv[1]));

      }
      else{
        if (execvp(pipe2->command.program, pipe2->command.argv) < 0){
          perror("execvp");
          exit(1);
        }
        exit(0);
      }
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(first_pid, NULL, 0);
    waitpid(second_pid, NULL, 0);
    return 0;
}

int execute_builtin(node_t* node){

  char* cmd = node->command.program;

  if(strcmp(cmd, "cd") == 0){
    chdir(node->command.argv[1]);
    return 0;
  }
  else if (strcmp(cmd, "exit") == 0){
    exit(atoi(node->command.argv[1]));
    return 0;
  }
  else{
    return 1;
  }

}

void execute_nonbuiltin(node_t* node){

  pid_t pid;

  pid = fork();
  if (pid == 0){
    if (execvp(node->command.program, node->command.argv) < 0){
      perror("execvp");
      exit(1);
    }
  }
  else{
    waitpid(pid, NULL, 0);
  }

}

void alarm_handler(int sig_num){

  printf("%s %d", "Interrupted: ", sig_num);

}

void initialize(void){
    /* This code will be called once at startup */
    if (prompt)
    {
      prompt = "vush$ ";
    }

}

void run_command(node_t *node)
{

    signal(SIGINT, alarm_handler);
      switch (node->type) {
        case NODE_SEQUENCE:{
          run_command(node->sequence.first);
          run_command(node->sequence.second);
          break;
        }
        case NODE_PIPE:{
          run_pipe(node);
          break;
        }
        case NODE_REDIRECT:{
          break;
        }
        case NODE_SUBSHELL:{
          pid_t pid;

          pid = fork();
          if (pid == 0){
            run_command(node->subshell.child);
          }
          else{
            waitpid(pid, NULL, 0);
          }
          break;
        }
        case NODE_DETACH:{
          break;
        }
        case NODE_COMMAND:{
          if (execute_builtin(node) != 0){
            execute_nonbuiltin(node);
          }
          break;
        }
        default:{
          //perror("Uknown error");
        }
      }

}
