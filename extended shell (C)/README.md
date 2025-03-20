# Advanced C Programming Lab: Shell Implementation

## Key C Programming Skills Demonstrated

### 1. Process Management
- **Process Creation and Control**
  - Mastered `fork()` system call for creating child processes
  - Implemented process creation and management using `pid_t` type
  - Utilized process control signals (`SIGINT`, `SIGTSTOP`, `SIGCONT`)
  - Managed process states (Running, Suspended, Terminated)

### 2. Inter-Process Communication
- **Pipe Implementation**
  - Created and managed pipes using `pipe()` system call
  - Redirected standard input/output between processes
  - Used `dup()` and `dup2()` for file descriptor manipulation
  - Implemented complex process communication pipelines

### 3. Advanced System Programming Techniques
- **Signal Handling**
  - Sent and managed process signals using `kill()` function
  - Implemented custom signal response mechanisms

- **Process Tracking**
  - Created a dynamic linked list to track running processes
  - Used `waitpid()` with `WNOHANG` for non-blocking process status checks

### 4. Memory Management
- **Dynamic Memory Allocation**
  - Managed memory for command line parsing
  - Implemented memory cleanup functions
  - Prevented memory leaks in process and command tracking

### 5. File Descriptor Manipulation
- **Low-Level I/O Operations**
  - Redirected standard input and output streams
  - Managed file descriptors for input/output redirection
  - Implemented pipeline input/output routing

### 6. Command Line Parsing and Handling
- **String Manipulation**
  - Parsed complex command lines with multiple arguments
  - Implemented history mechanism with command line storage
  - Handled different command types (built-in vs. executable)

### 7. Linked List Implementation
- **Data Structure Management**
  - Created a custom linked list for process tracking
  - Implemented list insertion, update, and deletion operations
  - Managed list memory efficiently

## Technologies and System Calls Used
- Process creation: `fork()`
- Process control: `kill()`
- I/O redirection: `dup()`, `dup2()`
- Process waiting: `waitpid()`
- Pipe creation: `pipe()`
- Execution: `execvp()`

## Learning Outcomes
This lab provided hands-on experience in:
- Unix/Linux system programming
- Low-level process and I/O management
- Advanced C programming techniques
- Shell implementation fundamentals

## Project Structure
- `mypipeline.c`: Standalone pipe implementation
- `myshell.c`: Advanced shell with process management and history

## Compilation
```bash
make
./mypipeline    # Pipe demonstration
./myshell       # Advanced shell
```

## Challenges Overcome
- Synchronization between processes
- Efficient memory management
- Complex I/O redirection
- Implementing a fully functional shell

## Skills Applicable to Real-World Software Development
- Systems programming
- Unix/Linux environment interaction
- Low-level performance optimization
- Robust error handling
