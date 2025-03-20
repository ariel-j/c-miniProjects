#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For getcwd(), fork(), execvp()
#include <linux/limits.h> // For PATH_MAX
#include "LineParser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#define INPUT_BUFFER_SIZE 2048


void execute(cmdLine *pCmdLine) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) { // Child process
        // Handle input redirection (<)
        if (pCmdLine->inputRedirect) {
            int inputFd = open(pCmdLine->inputRedirect, O_RDONLY);
            if (inputFd == -1) {
                perror("Failed to open input file");
                exit(1);
            }
            dup2(inputFd, STDIN_FILENO);  // Redirect input
            close(inputFd);  // Close file descriptor
        }

        // Handle output redirection (>)
        if (pCmdLine->outputRedirect) {
            int outputFd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (outputFd == -1) {
                perror("Failed to open output file");
                exit(1);
            }
            dup2(outputFd, STDOUT_FILENO);  // Redirect output
            close(outputFd);  // Close file descriptor
        }

        // Execute the command
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        perror("execvp failed");
        exit(1);
    } else { // Parent process
        if (pCmdLine->blocking) {
            waitpid(pid, NULL, 0); // Wait for the child process to finish
        }
    }
}

int main() {
    char input[INPUT_BUFFER_SIZE];
    cmdLine *parsedLine;
    char cwd[PATH_MAX];

    while (1) {
        // Display the prompt
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);  // Show current working directory
        } else {
            perror("getcwd failed");
            exit(EXIT_FAILURE);
        }

        // Read user input
        if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
            perror("Error reading input");
            continue; // Retry loop on failure
        }

        // Parse the input
        parsedLine = parseCmdLines(input);
        if (!parsedLine) continue;  // Skip if input is empty or invalid

        // Handle "quit" command
        if (strcmp(parsedLine->arguments[0], "quit") == 0) {
            freeCmdLines(parsedLine);
            break; // Exit the shell
        }

        // Execute the command
        execute(parsedLine);

        // Free parsed command resources
        freeCmdLines(parsedLine);
    }

    return 0;
}
