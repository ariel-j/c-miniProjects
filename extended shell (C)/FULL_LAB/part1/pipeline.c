#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


// For easier readability.
#define SUCCESS 1
#define FAILURE 0

int child1Execute(pid_t child1, int fd[2]){
    if(child1 < 0){
        perror("fork error in child1");
        return FAILURE;
    }

    if(child1 == 0){  

        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");

        close(STDOUT_FILENO);
        dup(fd[1]);
        close(fd[1]);
        char* arg_list[] = {"ls", "-l", NULL};

        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");

        execvp("ls",arg_list);
        perror("execvp error in child1");
        exit(EXIT_FAILURE);
    }
    return SUCCESS;  
}

int child2Execute(pid_t child2, int fd[2]){
    if(child2 < 0){
            perror("fork error in child2");
            return FAILURE;
        }

    if(child2 == 0){

        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");

        close(STDIN_FILENO);
        dup(fd[0]);
        close(fd[0]);
        char* arg_list[] = {"tail", "-n", "2", NULL};

        fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");

        execvp("tail",arg_list);
        perror("execvp error");
        exit(EXIT_FAILURE);
    }
    return SUCCESS;
}

void closeWriteEnd(int fd[2]){
        fprintf(stderr,  "(parent_process>closing the write end of the pipe…)\n");
        close(fd[1]);
}

void closeReadEnd(int fd[2]){
        fprintf(stderr,  "(parent_process>closing the read end of the pipe…)\n");
        close(fd[0]);
}

void waitChildren(pid_t child1, pid_t child2){

        fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");

        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
}

int main(int argc, char** argv){
    int fd[2]; // fd[0] = read, fd[1] = write
    pid_t child1,child2;

    if(pipe(fd) == -1) {
        perror("pipe error");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "(parent_process>forking child1)\n");
    child1 = fork();
    if(!child1Execute(child1,fd)){
        closeWriteEnd(fd);
        return EXIT_FAILURE;
    }
    else {
        closeWriteEnd(fd);
        fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);
        fprintf(stderr, "(parent_process>forking child2)\n");
        child2 = fork();
        if(!child2Execute(child2,fd)){
            closeReadEnd(fd);
            return EXIT_FAILURE;
        }
        fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);        
        closeReadEnd(fd);
        waitChildren(child1,child2);
    }
    fprintf(stderr, "(parent_process>exiting...)\n");
    return EXIT_SUCCESS;
}

/**ANSWERS
 * 
 * 
 * =================== ORIGINAL OUTPUT ===================
 * 
(parent_process>forking child1)
(parent_process>closing the write end of the pipe…)
(child1>redirecting stdout to the write end of the pipe...)
(child1>going to execute cmd: ls -l)
(parent_process>created process with id: 4121)
(parent_process>forking child2)
(parent_process>created process with id: 4122)
(parent_process>closing the read end of the pipe…)
(parent_process>waiting for child processes to terminate...)
(child2>redirecting stdin to the read end of the pipe...)
(child2>going to execute cmd: tail -n 2)
-rw-r--r-- 1 nemiku nemiku  2912 Dec 13 11:02 pipeline.c
-rw-r--r-- 1 nemiku nemiku  9528 Dec 13 11:03 pipeline.o
(parent_process>exiting...)



 * 
 * 
 * 
 *  
 * =================== Not closing ReadEnd in Parent Process ===================
(parent_process>forking child1)
(parent_process>created process with id: 4590)
(parent_process>forking child2)
(child1>redirecting stdout to the write end of the pipe...)
(child1>going to execute cmd: ls -l)
(parent_process>created process with id: 4591)
(parent_process>closing the read end of the pipe…)
(parent_process>waiting for child processes to terminate...)
(child2>redirecting stdin to the read end of the pipe...)
(child2>going to execute cmd: tail -n 2)
<wait>

WHY? 
because the pipe write end is remained opened in the parent process, 
thus the tail -n 2 command will wait indefinitely until more input is coming
*
*
*
*
* =================== Not closing ReadEnd in Parent Process ===================
(parent_process>forking child1)
(parent_process>closing the write end of the pipe…)
(child1>redirecting stdout to the write end of the pipe...)
(parent_process>created process with id: 5802)
(parent_process>forking child2)
(child1>going to execute cmd: ls -l)
(parent_process>created process with id: 5803)
(parent_process>waiting for child processes to terminate...)
(child2>redirecting stdin to the read end of the pipe...)
(child2>going to execute cmd: tail -n 2)
-rw-r--r-- 1 nemiku nemiku  4158 Dec 13 11:10 pipeline.c
-rw-r--r-- 1 nemiku nemiku  9144 Dec 13 11:10 pipeline.o
(parent_process>exiting...)


WHY? 
it seems quite the same -> while the read end of the pipe is remained open in the parent process.
probably because execvp is handling those descriptors well.



* =================== Not closing WriteEnd in Parent Process + not waiting for termination of children===================
(parent_process>forking child1)
(parent_process>created process with id: 6873)
(child1>redirecting stdout to the write end of the pipe...)
(parent_process>forking child2)
(child1>going to execute cmd: ls -l)
(parent_process>created process with id: 6874)
(child2>redirecting stdin to the read end of the pipe...)
(parent_process>closing the read end of the pipe…)
(parent_process>exiting...)
(child2>going to execute cmd: tail -n 2)


WHY?
in contrast to the first time when we only didn't close the read end ->
 now we don't wait indefinitely in the tail command because we don't wait for the children to finish
 */