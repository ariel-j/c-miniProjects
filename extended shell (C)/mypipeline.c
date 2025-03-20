#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void myPipeLine() {
    // Create a pipe
    int pipefd[2];
    pid_t pid1, pid2;

    fprintf(stderr, "(parent_process>forking…)\n");
    if (pipe(pipefd) == -1) {
        perror("pipe error");
        exit(1);
    }

    // Fork a first child process (child1)
    pid1 = fork();
    if (pid1 == -1) {
        perror("fork1 error");
        exit(1);
    }

    if (pid1 == 0) { // In the child1 process:
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");

        // Close the standard output.
        close(STDOUT_FILENO);

        // Duplicate the write-end of the pipe using dup (see man).
        dup(pipefd[1]);

        // Close the file descriptor that was duplicated.
        close(pipefd[1]);
        close(pipefd[0]); // ??? do i need to close it here?

        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        // Execute "ls -l".
        if (execlp("ls", "ls", "-l", (char *)NULL) == -1) {
            perror("execlp failed");  // Print error if execlp fails
            exit(1);
        }
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", pid1);

    // In the parent process: Close the write end of the pipe.
    fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
    close(pipefd[1]);

    // Fork a second child process (child2).
    fprintf(stderr, "(parent_process>forking…)\n");
    pid2 = fork();
    if (pid2 == -1) {
        perror("fork2 error");
        exit(1);
    }

    if (pid2 == 0) { // In the child2 process:
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");

        // Close the standard input.
        close(STDIN_FILENO);

        // Duplicate the read-end of the pipe using dup.
        dup(pipefd[0]);

        // Close the file descriptor that was duplicated.
        close(pipefd[0]);

        fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");
        // Execute "tail -n 2".
        if (execlp("tail", "tail", "-n", "2", NULL) == -1) {
            perror("execlp failed");
            exit(1);
        }
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", pid2);

    // In the parent process: Close the read end of the pipe.
    fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
    close(pipefd[0]);

    // Now wait for the child processes to terminate, in the same order of their execution.
    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(pid1, NULL, 0);  // Wait for child1
    waitpid(pid2, NULL, 0);  // Wait for child2

    fprintf(stderr, "(parent_process>exiting…)\n");
}

int main() {
    myPipeLine();
    return 0;
}
