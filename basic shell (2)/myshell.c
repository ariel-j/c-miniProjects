#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include "LineParser.h"
int debug = 0;

/** 
 *   1.Display a prompt - the current working directory (see man getcwd). The path name is not expected to exceed PATH_MAX (it's defined in linux/limits.h, so you'll need to include it).
 */
void displayPrompt(){
    char cwd[PATH_MAX];
    if(getcwd(cwd,sizeof(cwd))!= NULL){
        printf ("%s>",cwd);
    }
    else {
        perror ("getcwd error");
    }
}

/** 
 *  2. Read a line from from stdin (no more than 2048 bytes). It is advisable to use fgets (see man).
 */
void readUserInput(char input[], int size){
    fgets(input,size,stdin);
}

/**
 * 3.Parse the input using parseCmdLines() (LineParser.h). The result is a structure cmdLine that contains all necessary parsed data.
 */
cmdLine * parseInput(char input[]){
    return parseCmdLines(input);
}


void handleInputRedirect(cmdLine *pCmdLine){
    FILE *input_file = fopen(pCmdLine->inputRedirect, "r");
    if(!input_file){
        perror("input redirection error");
        _exit(1);
    }
    dup2(fileno(input_file), STDIN_FILENO); //fileno
    fclose(input_file);
}

void handleOutputRedirect(cmdLine *pCmdLine){
    FILE *output_file = fopen(pCmdLine->outputRedirect, "w");
    if(!output_file){
        perror("output redirection error");
        _exit(1);
    }
    dup2(fileno(output_file), STDOUT_FILENO); //fileno
    fclose(output_file);
}

/**
 * 4.Write a function execute(cmdLine *pCmdLine) that receives a parsed line and invokes the program specified in the cmdLine using the proper system call (see man execv).
 * 5.Use perror (see man) to display an error if the execv fails, and then exit "abnormally".
 *
 * execv is replacing the current process(the shell) with a new process - meaning the code after execv in the shell will not be executed as the process is replaced
 * execv requires the full path thus "ls" will not work as it's not a full path
 * execvp - search the directories listed in the PATH... so ls will work
 * because the * is wildcard expanded by the shell BUT we are not using the shell when writing ls * we are using the child process
*/ 

void execute(cmdLine *pCmdLine){

    pid_t pid = fork(); // Creating new child process (so we won't lose the shell process :))
    if(pid == -1){ 
        perror("fork error - failed to create new process");
        exit(1); // NEEDS TO TERMINATE SHELL THUS regular exit
    }

    if(pid == 0){
        if(debug){
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }

        if(pCmdLine->inputRedirect){
            handleInputRedirect(pCmdLine);
        }

        if(pCmdLine->outputRedirect){
            handleOutputRedirect(pCmdLine);
        }

        if(execvp(pCmdLine->arguments[0],pCmdLine->arguments) == -1){
            perror("execvp error");
            _exit(1);
        }
        
    } else {
        if (pCmdLine->blocking) { // wait for child process to finish - & was added to so it's blocking.
            if (debug) {
                fprintf(stderr, "PID: %d\n", pid);
                fprintf(stderr, "Waiting for child process to finish\n");
            }
            waitpid(pid, NULL, 0);
        } else {
            if (debug) {
                fprintf(stderr, "PID: %d\n", pid);
                fprintf(stderr, "Running in background\n");
            }
        }
    }
}

void isDebugMode(int argc, char **argv){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"-d") == 0){
            debug = 1;
            break;
        }
    }
}

void checkQuit(cmdLine* command){
        freeCmdLines(command);
		exit(0);
}

void checkCD (cmdLine* command){
        if(command->argCount < 2 && debug){
            fprintf(stderr, "cd: missing argument\n");
            
        } else {
            if(chdir(command->arguments[1]) != 0 && debug){
                fprintf(stderr,"cd error\n");
            }
        }
}

void stop_process(int pid) {
    if(kill(pid, SIGSTOP) == -1)
        perror("stop error");
}

void wake_process(int pid) {
    if(kill(pid, SIGCONT) == -1)
        perror("wake error");
}

void term_process(int pid) {
    if(kill(pid, SIGINT) == -1)
        perror("term error");
}

void checkStop(cmdLine* command){
    if(command->argCount > 1){
        stop_process(atoi(command->arguments[1]));
    }
}

void checkWake(cmdLine* command){
    if(command->argCount > 1){
        wake_process(atoi(command->arguments[1]));
    }
}

void checkTerm(cmdLine* command){
    if(command->argCount > 1){
        term_process(atoi(command->arguments[1]));
    }
}

int checkCommand(cmdLine* command){
    if(strcmp(command->arguments[0],"quit") == 0){checkQuit(command);return 1;}
    if(strcmp(command->arguments[0], "cd") == 0){checkCD(command); return 1;}
    if(strcmp(command->arguments[0], "stop") == 0){checkStop(command); return 1;}
    if(strcmp(command->arguments[0], "wake") == 0){checkWake(command); return 1;}
    if(strcmp(command->arguments[0], "term") == 0){checkTerm(command); return 1;}
    return 0;
}

int main(int argc, char **argv)
{
    signal(SIGINT, SIG_IGN);
	int size = 2048;
    char input[size];
    cmdLine* command;
	while (1)
	{
		displayPrompt();
        readUserInput(input,size);
        isDebugMode(argc,argv);
        if(input [0] == '\n') continue; // skip empty input.
        command = parseInput(input);
        if(command == NULL) continue; // skip null commands.
        if(checkCommand(command)) continue;
        execute(command);
        freeCmdLines(command);
		sleep(1);
	}

	return 0;
}