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
#include <fcntl.h>
#include <errno.h>

//process manager
    #define TERMINATED  -1
    #define RUNNING 1
    #define SUSPENDED 0
    #define HISTLEN 10
    #define MAX_JOBS 100
    int debug = 0;
     typedef struct process{
        cmdLine* cmd;                         /* the parsed command line*/
        pid_t pid; 		                  /* the process id that is running the command*/
        int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
        struct process *next;	                  /* next process in chain */
    } process;
    process* process_list;

    void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
    void removeProcess(pid_t pid);  
    void updateProcessList(process **process_list);
    void freeProcessList(process* process_list);
    void updateProcessStatus(process* process_list, int pid, int status);
    void printProcessList(process** process_list);
    void checkProcs(cmdLine* command);

// ================ PART 4 ================
typedef struct history_node {
    char* command;               
    struct history_node* next;
    struct history_node* prev;  
} history_node;

typedef struct history_list {
    history_node* head;      // oldest command     
    history_node* tail;           
    int size;                     
} history_list;
history_list * history_global = NULL;

history_list* init_history() {
    history_list* hist = (history_list*)malloc(sizeof(history_list));
    hist->head = NULL;
    hist->tail = NULL;
    hist->size = 0;
    return hist;
}

char* copyString(const char* source) {
    char* copy = (char*)malloc(strlen(source) + 1);  // +1 for null terminator
    strcpy(copy, source);
    return copy;
}

void add_to_empty_history(history_list* hist, history_node* new_node){
    hist->head = new_node;
    hist->tail = new_node;
    hist->size = 1;
}

void remove_oldest_command(history_list* hist){
    history_node* old_head = hist->head;
    hist->head = hist->head->next;
    hist->head->prev = NULL;  
    free(old_head->command);
    free(old_head);
    hist->size--;
}

void add_to_non_empty_history(history_list* hist, history_node* new_node){
    hist->tail->next = new_node;
    new_node->prev = hist->tail;  
    hist->tail = new_node;
    hist->size++;
    
    if (hist->size > HISTLEN) {
        remove_oldest_command(hist);
    }
}

void add_to_history(history_list* hist, const char* command) {
    if (!command || command[0] == '\n') return;  
    history_node* new_node = (history_node*)malloc(sizeof(history_node));
    new_node->command = copyString(command);
    new_node->next = NULL;
    new_node->prev = NULL;       
    
    if (hist->size == 0) {
        add_to_empty_history(hist, new_node);
        return;
    }
    
    add_to_non_empty_history(hist, new_node);
}

void print_history(history_list* hist) {
    history_node* current = hist->head;
    int index = 1;
    
    while (current) {
        printf("%d  %s", index++, current->command);
        if (current->command[strlen(current->command)-1] != '\n') {
            printf("\n");
        }
        current = current->next;
    }
}

char* get_command_by_index(history_list* hist, int index) {
    if (index < 1 || index > hist->size) {
        return NULL;
    }
    
    history_node* current = hist->head;
    for (int i = 1; i < index; i++) {
        current = current->next;
    }
    return current->command;
}

char* get_last_command(history_list* hist) {
    if (!hist->tail) return NULL;

    history_node* current = hist->tail;
    while (current) {
        if (strncmp(current->command, "!!", 2) != 0 && 
            strncmp(current->command, "history", 7) != 0 && 
            current->command[0] != '!') {
            return current->command;
        }
        current = current->prev;  
    }
    return NULL;
}

void free_history(history_list* hist) {
    history_node* current = hist->head;
    while (current) {
        history_node* next = current->next;
        free(current->command);
        free(current);
        current = next;
    }
    free(hist);
}

int check_last_command(history_list* hist, char* input){
    char* last_cmd = get_last_command(hist);
    if (!last_cmd) {
        printf("No commands in history\n");
        return 1;
    }
    printf("%s", last_cmd);
    strcpy(input, last_cmd);
    return 0; 
}

int check_index_command(history_list* hist, char* input){
    int index = atoi(input + 1);
    char* cmd = get_command_by_index(hist, index);
    if (!cmd) {
        printf("Invalid history index\n");
        return 1;
    }
    printf("%s", cmd);
    strcpy(input, cmd);
    return 0;
}

int check_history_command(char* input, history_list* hist) {
    if (strcmp(input, "history\n") == 0) {
        print_history(hist);
        return 1;
    }
    if (strcmp(input, "!!\n") == 0) {
        return check_last_command(hist, input);
    }
    if (input[0] == '!') {
        return check_index_command(hist, input);
    }
    return 0;
}

//shell functions
void displayPrompt(){
    char cwd[PATH_MAX];
    if(getcwd(cwd,sizeof(cwd))!= NULL){
        printf ("%s>",cwd);
    }
    else {
        perror ("getcwd error");
    }
}

void readUserInput(char input[], int size){
    fgets(input,size,stdin);
}

cmdLine * parseInput(char input[]){
    return parseCmdLines(input);
}

//handler
void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        removeProcess(pid);  // Remove process from the job list
        process* current = process_list; // Update process list status to terminated
        while (current != NULL) {
            if (current->pid == pid) {
                if (WIFEXITED(status)) {
                    current->status = TERMINATED;  // Mark the process as terminated
                } else if (WIFSIGNALED(status)) {
                    current->status = TERMINATED;  // Mark as terminated if killed by a signal
                }
                break;
            }
            current = current->next;
        }
    }
}

void handleInputRedirect(cmdLine *pCmdLine){
    FILE *input_file = fopen(pCmdLine->inputRedirect, "r");
    if(!input_file){
        perror("input redirection error");
        _exit(1);
    }
    dup2(fileno(input_file), STDIN_FILENO);
    fclose(input_file);
}

void handleOutputRedirect(cmdLine *pCmdLine){
    FILE *output_file = fopen(pCmdLine->outputRedirect, "w");
    if(!output_file){
        perror("output redirection error");
        _exit(1);
    }
    dup2(fileno(output_file), STDOUT_FILENO);
    fclose(output_file);
}

//execute
void executePipe(cmdLine *pCmdLine) {
    cmdLine *leftCmd = pCmdLine;
    cmdLine *rightCmd = pCmdLine->next;

    // Validate I/O redirections for pipe
    if (leftCmd->outputRedirect || rightCmd->inputRedirect) {
        fprintf(stderr, "Error: Cannot redirect output of left process or input of right process in a pipeline\n");
        return;
    }

    int pipefd[2];
    pid_t pid1, pid2;

    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe error");
        return;
    }

    // Fork first child (left side of pipe)
    pid1 = fork();
    if (pid1 == -1) {
        perror("fork error - left process");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid1 == 0) {  // Left child process
        if (debug) {
            fprintf(stderr, "Left Child PID: %d\n", getpid());
            fprintf(stderr, "Executing left command: %s\n", leftCmd->arguments[0]);
        }

        // Close read end of pipe
        close(pipefd[0]);

        // Redirect stdout to write end of pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2 error for left process");
            _exit(1);
        }

        // Close write end of pipe
        close(pipefd[1]);

        // Handle input redirection if present
        if (leftCmd->inputRedirect) {
            handleInputRedirect(leftCmd);
        }

        // Execute left command
        if (execvp(leftCmd->arguments[0], leftCmd->arguments) == -1) {
            perror("execvp error for left command");
            _exit(1);
        }
    }

    // Fork second child (right side of pipe)
    pid2 = fork();
    if (pid2 == -1) {
        perror("fork error - right process");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid2 == 0) {  // Right child process
        if (debug) {
            fprintf(stderr, "Right Child PID: %d\n", getpid());
            fprintf(stderr, "Executing right command: %s\n", rightCmd->arguments[0]);
        }

        // Close write end of pipe
        close(pipefd[1]);

        // Redirect stdin to read end of pipe
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2 error for right process");
            _exit(1);
        }

        // Close read end of pipe
        close(pipefd[0]);

        // Handle output redirection if present
        if (rightCmd->outputRedirect) {
            handleOutputRedirect(rightCmd);
        }

        // Execute right command
        if (execvp(rightCmd->arguments[0], rightCmd->arguments) == -1) {
            perror("execvp error for right command");
            _exit(1);
        }
    }

    // Parent process
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for child processes if blocking is requested
    if (leftCmd->blocking && rightCmd->blocking) {
        if (debug) {
            fprintf(stderr, "Waiting for both child processes\n");
        }
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        if (debug) {
            fprintf(stderr, "Running in background mode\n");
        }
    }
}


void runChildProcess(cmdLine *pCmdLine){
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
}

void runParentProcess(cmdLine *pCmdLine, pid_t pid){
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

void execute(cmdLine *pCmdLine){
    // Check if this is a pipe command
    if (pCmdLine->next) {
        executePipe(pCmdLine);
    }

    pid_t pid = fork(); 
    if(pid == -1){ 
        perror("failed to create new process");
        exit(1);
    }

    if(pid == 0){
       runChildProcess(pCmdLine);
        
    } else {
        addProcess(&process_list, pCmdLine, pid);  // Add to the process list
        runParentProcess(pCmdLine, pid);
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
        freeProcessList(process_list);
        free_history(history_global);
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

//procecess checks
void stop_process(int pid) {
    if(kill(pid, SIGSTOP) == -1)
        perror("stop error");
    else
        updateProcessStatus(process_list, pid, SUSPENDED);
}

void wake_process(int pid) {
    if(kill(pid, SIGCONT) == -1)
        perror("wake error");
    else
        updateProcessStatus(process_list, pid, RUNNING);
}

void term_process(int pid) {
    if(kill(pid, SIGINT) == -1)
        perror("term error");
    else
        updateProcessStatus(process_list, pid, TERMINATED);
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
    if(strcmp(command->arguments[0], "procs") == 0){checkProcs(command); return 1;} //Part3A

    return 0;
}

void checkProcs(cmdLine* command){
    printProcessList(&process_list);
}

//process list functions
void addProcess(process** process_list, cmdLine* cmd, pid_t pid) {
    process* new_process = (process*) malloc(sizeof(process));
    new_process->cmd = parseCmdLines(cmd->arguments[0]);
    if (new_process->cmd == NULL) {
        free(new_process);
        return;
    }
    // Replacing the cmdline with newString
    for (int i = 1; i < cmd->argCount; i++) {
        replaceCmdArg(new_process->cmd, i, cmd->arguments[i]);
    }
    //printf("%s\t,%s\n","the argument is",new_process->cmd->arguments[0]);
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;

    if (debug) {
        fprintf(stderr, "DEBUG: Added process PID=%d, Command=%s\n", pid, cmd->arguments[0]);
    }
}

void printProcessList(process** process_list) {
    updateProcessList(process_list);
    printf("\nPID          Command      STATUS\n");
    process* current = *process_list;
    process* prev = NULL;
    while (current != NULL) {
        // Print the process information
        printf("%-12d %-12s %-12s\n",
               current->pid,
               current->cmd->arguments[0],
               current->status == RUNNING ? "Running" :
               (current->status == SUSPENDED ? "Suspended" : "Terminated"));

        // Check if the process is terminated
        if (current->status == TERMINATED) {
            process* to_delete = current;  // Save the node to delete
            if (prev == NULL) {
                *process_list = current->next;  // Update head if the first node is deleted
            } else {
                prev->next = current->next;  
            }
            current = current->next; 
            freeCmdLines(to_delete->cmd);  
            free(to_delete);  // Free the process struct
        } else {
            prev = current;       
            current = current->next;  
        }
    }
}

void removeProcess(pid_t pid) {
    process* current = process_list;
    process* previous = NULL;

    while (current != NULL) {
        if (current->pid == pid) {
            if (previous == NULL) {
                process_list = current->next;
            } else {
                previous->next = current->next;
            }
            free(current); 
            return;
        }
        previous = current;
        current = current->next;
    }
    fprintf(stderr, "Error: No process found with pid %d\n", pid);
}

void freeProcessList(process* process_list) {
    process* current = process_list;
    while (current != NULL) {
        process* next = current->next;  // Store the next process
        freeCmdLines(current->cmd);    // Free the parsed command line
        free(current);                 
        current = next;                
    }
}

void updateProcessStateChanged(int status,process* curr){
    if (WIFSTOPPED(status)) {
        curr->status = SUSPENDED;
    } else if (WIFCONTINUED(status)) {
        curr->status = RUNNING;
    } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        curr->status = TERMINATED;
    }
}

void updateProcessList(process **process_list) {
    if(!process_list || !*process_list) return;
    process* current = *process_list;
    process* prev = NULL;
    while (current) {
        int status;
        pid_t result = waitpid(current->pid, &status, WNOHANG);
        if (result > 0) { // process has changed state
           updateProcessStateChanged(status,current);
        }
        if (current->status == TERMINATED) { // Remove terminated processes
            process* to_delete = current;
            if (prev == NULL) {
                *process_list = current->next; // Update head
            } else {
                prev->next = current->next;
            }
            current = current->next;
            freeCmdLines(to_delete->cmd);
            free(to_delete);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void updateProcessStatus(process* process_list, int pid, int status) {
    process* current = process_list;
    while (current != NULL) {
        if (current->pid == pid) {
            current->status = status; 
            return;
        }
        current = current->next;  
    }
}

//main helper functions: 
void handleProcsCommand(process* process_list) {
    printProcessList(&process_list);  // Print the list of processes
}

int handleUserInput(char input[], cmdLine** command, int argc, char** argv, process** process_list) {
    readUserInput(input, 2048);  
    isDebugMode(argc, argv);     
    if(input[0] == '\n') return 0;  // Skip empty input
    if (check_history_command(input, history_global)) return 0;
    add_to_history(history_global, input);
    *command = parseInput(input);
    if (*command == NULL) return 0;  // Skip null commands
    return 0;  
}

void executeCommand(cmdLine* command, process** process_list) {
    if (checkCommand(command)) {
        freeCmdLines(command);
    } else {
        execute(command);
        freeCmdLines(command);
    }
}

int main(int argc, char **argv) {
    signal(SIGINT, SIG_IGN);
    char input[2048];
    cmdLine* command;
    history_global = init_history();
    process_list = NULL;  

    while (1) {
        displayPrompt();
        if (handleUserInput(input, &command, argc, argv, &process_list)) {
            continue;  // Skip command execution if "procs" command was processed
        }
        executeCommand(command, &process_list);
        sleep(1);  // Sleep to avoid CPU overload
    }
    freeProcessList(process_list);
    return 0;
}   