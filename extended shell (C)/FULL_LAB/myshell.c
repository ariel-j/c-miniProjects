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


#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 10
int debug = 0;

// ================ MOST PARTS OF THIS CODE WERE TAKEN FROM LAB2 ====================
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

// ================ PART 3 ================

//Create a linked list to store information about running/suspended processes. Each node in the list is a struct process:
typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;
process* process_list;

void freeProcess (process* process){
    if(!process) return;
    freeCmdLines(process->cmd); // clean cmd Struct
    free(process); // clean process Struct
}

/**
 * free all memory allocated for the process list.
 */
void freeProcessList(process* process_list){
    while(process_list){
        process* next = process_list->next;
        freeProcess(process_list);
        process_list = next;
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

/**
 * go over the process list, and for each process check if it is done, 
 * you can use waitpid with the option WNOHANG. WNOHANG does not block the calling process,
 * the process returns from the call to waitpid immediately. 
 * If no process with the given process id exists, then waitpid returns -1.
 * READ WAITPID(2)
 */
void updateProcessList(process **process_list){
    if(!process_list || !*process_list) return;
    process* curr = *process_list;
    while(curr){
        int status;
        pid_t result = waitpid(curr->pid, &status, WNOHANG | WUNTRACED);
        if (result > 0){ // Process State changed
            updateProcessStateChanged(status,curr);
        }
        else if (result == 0) // Process State didn't change
            curr -> status = RUNNING;
        else // Process doesn't exist - terminated.
            curr -> status = TERMINATED;

        curr = curr->next;
    }
}

char* getStatusString(int status) {
    switch(status) {
        case TERMINATED:
            return "Terminated";
        case RUNNING:
            return "Running";
        case SUSPENDED:
            return "Suspended";
        default:
            return "Unknown";
    }
}

/**
 * Receive a process list (process_list),
 * a command (cmd), 
 * and the process id (pid) of the process running the command. 
 * Note that process_list is a pointer to a pointer so 
 * that we can insert at the beginning of the list if we wish. <----- we insert it in the beginning.
 */
void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_process = (process*)malloc(sizeof(process));

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
}

void removeTerminatedProcess(process* prev, process** current, process** process_list){
    if(prev){ // there's a previous process
        prev->next = (*current)->next;
        freeProcess(*current);
        *current = prev->next;
    }
    else { // no previous process
        *process_list = (*current)->next;
        freeProcess(*current);
        *current = *process_list;
    }
}

/**
 * print the processes.
 */
void printProcessList(process** process_list){
    updateProcessList(process_list);
    printf("PID\tCommand\t\tSTATUS\n");

    if(process_list != NULL) {
        process* current = *process_list;
        process* prev = NULL;

        while(current){
            printf("%d\t%s\t\t%s\n",
                current->pid,
                current->cmd->arguments[0],
                getStatusString(current->status));

            if(current->status == TERMINATED){
                removeTerminatedProcess(prev, &current, process_list);
            }
            else{
                prev = current;
                current = current->next;
            }
        }
    }
}

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

/**
 * 4.Write a function execute(cmdLine *pCmdLine) that receives a parsed line and invokes the program specified in the cmdLine using the proper system call (see man execv).
 * 5.Use perror (see man) to display an error if the execv fails, and then exit "abnormally".
 *
 * execv is replacing the current process(the shell) with a new process - meaning the code after execv in the shell will not be executed as the process is replaced
 * execv requires the full path thus "ls" will not work as it's not a full path
 * execvp - search the directories listed in the PATH... so ls will work
 * because the * is wildcard expanded by the shell BUT we are not using the shell when writing ls * we are using the child process
*/ 
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
//=========================== TOOK FROM PART 1 =============================================

int child1Execute(pid_t child1, int fd[2], cmdLine *leftCmd) {
    if (child1 < 0) {
        perror("fork error in child1");
        return 0;
    }

    if (child1 == 0) {
        if (debug) {
            fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        }
        close(STDOUT_FILENO);
        dup(fd[1]);
        close(fd[1]);
        close(fd[0]);

        runChildProcess(leftCmd);
    }
    return 1;
}

int child2Execute(pid_t child2, int fd[2], cmdLine *rightCmd) {
    if (child2 < 0) {
        perror("fork error in child2");
        return 0;
    }

    if (child2 == 0) {
        if (debug) {
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
        }
        close(STDIN_FILENO);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);

        runChildProcess(rightCmd);
    }
    return 1;
}

void executePipeline(cmdLine *leftCmd, cmdLine *rightCmd) {
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe error");
        return;
    }

    pid_t child1 = fork();
    if (!child1Execute(child1, fd, leftCmd)) {
        close(fd[0]);
        close(fd[1]);
        return;
    }

    pid_t child2 = fork();
    if (!child2Execute(child2, fd, rightCmd)) {
        close(fd[0]);
        close(fd[1]);
        return;
    }

    close(fd[0]);
    close(fd[1]);

    if (debug) {
        fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
    }
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
}

//=========================== FINISHED TAKING FROM PART 1 =============================================

void executeSingle(cmdLine *pCmdLine){
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork error");
        exit(1);
    }

    if (pid == 0) runChildProcess(pCmdLine);
    else {
        addProcess(&process_list, pCmdLine, pid); 
        runParentProcess(pCmdLine, pid);
    }
}

int checkInvalidRedirection(cmdLine *pCmdLine){
    if (pCmdLine->outputRedirect || pCmdLine->next->inputRedirect) {
            fprintf(stderr, "Error: Invalid redirection with pipeline\n");
            return 0;
        }
    return 1;
}

void execute(cmdLine *pCmdLine) {
    if (pCmdLine->next && checkInvalidRedirection(pCmdLine)) 
        executePipeline(pCmdLine, pCmdLine->next); // executing pipeline command

    else 
        executeSingle(pCmdLine); // executing single command
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

void updateProcessStatus(process* process_list, int pid, int status) {
    while (process_list) {
        if (process_list->pid == pid) {
            process_list->status = status;
            return;
        }
        process_list = process_list->next;
    }
}

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

void checkProcs(cmdLine* command){
    printProcessList(&process_list);
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

int main(int argc, char **argv)
{
    signal(SIGINT, SIG_IGN);
	int size = 2048;
    char input[size];
    cmdLine* command;
    history_global = init_history();
	while (1)
	{
		displayPrompt();
        readUserInput(input,size);
        isDebugMode(argc,argv);
        if(input [0] == '\n') continue; // skip empty input.
        if (check_history_command(input, history_global)) continue;
        add_to_history(history_global, input);
        command = parseInput(input);
        if(command == NULL) continue; // skip null commands.
        if(checkCommand(command)) continue;
        execute(command);
        freeCmdLines(command);
		sleep(1);
	}
    free_history(history_global);
	return 0;
}