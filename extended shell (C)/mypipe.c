#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int main() {
    int pipefd[2];
    pid_t pid;
    char message[] = "hello";
    char buffer[100];

    if(pipe(pipefd) == -1){
        perror("pipe error");
        return 1;
    }

    pid = fork();
    if(pid == -1){
        perror("fork error");
        return 1;
    }

    if(pid == 0){  
        close(pipefd[0]);  
        write(pipefd[1], message, strlen(message)+1);
        close(pipefd[1]);
        exit(0);
    } else {  
        close(pipefd[1]);  
        read(pipefd[0], buffer, sizeof(buffer));
        printf("Received message: %s\n", buffer);
        close(pipefd[0]);
    }

    return 0;
}