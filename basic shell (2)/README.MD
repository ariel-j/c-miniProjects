# Shell Implementation Project

## Overview
This project implements a custom Linux shell that demonstrates fundamental concepts of operating systems, including process creation, signal handling, I/O redirection, and inter-process communication. The implementation provides insight into how command interpreters work in Unix-like systems and showcases key system programming techniques.

The shell supports command execution, directory navigation, background processes, process management via signals, and I/O redirection. Additionally, the project includes a pipe demonstration program to illustrate inter-process communication.

## Components

### 1. MyShell - Command Interpreter
A fully functional command-line shell with the following features:
- Command execution with path resolution
- Background process execution using `&`
- Working directory management with `cd` command
- Process management commands (`stop`, `wake`, `term`)
- Input/output redirection (`<`, `>`)
- Debug mode to show process information

### 2. MyPipe - Inter-Process Communication Demo
A separate program demonstrating the use of pipes for inter-process communication:
- Creates a parent-child process relationship
- Establishes a pipe connection between processes
- Sends data from child to parent process
- Demonstrates basic IPC principles

## Building the Project

To build the project, simply use the provided makefile:

```bash
# Build the shell
make myshell

# Build the pipe demo
make mypipe

# Build both
make
```

## Usage

### MyShell

Run the shell with:

```bash
./myshell          # Normal mode
./myshell -d       # Debug mode (shows PID and command info)
```

#### Basic Commands

```bash
/bin/ls            # Run a command with full path
ls                 # Run a command using PATH resolution
cat file.txt &     # Run a command in the background
cd directory       # Change current working directory
```

#### Process Management

```bash
stop <pid>         # Pause a running process (SIGSTOP)
wake <pid>         # Resume a paused process (SIGCONT)
term <pid>         # Terminate a process (SIGINT)
```

#### I/O Redirection

```bash
cat < input.txt    # Input redirection
ls > output.txt    # Output redirection
cat < in.txt > out.txt  # Both input and output redirection
```

#### Exiting the Shell

```bash
quit               # Exit the shell
```

### MyPipe

Run the pipe demo with:

```bash
./mypipe
```

This program creates a child process that sends a message to the parent process through a pipe. The parent process then displays the received message.

## Implementation Details

### Shell Architecture

The shell follows these main steps:
1. Display prompt (current working directory)
2. Read command from user
3. Parse command using LineParser library
4. Execute command by creating a child process 
5. Manage process execution (foreground/background)
6. Handle built-in commands directly

### Key System Calls Used

- `fork()`: Creates a new process by duplicating the calling process
- `execvp()`: Replaces the current process image with a new process image
- `waitpid()`: Suspends execution of the calling process until a child changes state
- `chdir()`: Changes the current working directory
- `kill()`: Sends a signal to a process or a group of processes
- `pipe()`: Creates a unidirectional data channel for inter-process communication

### Signal Handling

The shell implements basic process management through signals:
- `SIGSTOP`: Pauses process execution
- `SIGCONT`: Resumes paused processes
- `SIGINT`: Terminates processes

### I/O Redirection

Redirection is implemented by:
1. Opening the specified files with appropriate flags
2. Using `dup2()` to redirect standard input/output file descriptors
3. Executing the command with redirected I/O
4. Ensuring file descriptors are properly closed

## Technical Notes

- The shell uses the provided LineParser library for command parsing
- Error handling is implemented with appropriate messages using `perror()`
- The implementation focuses on demonstrating OS concepts rather than being a full-featured shell
- The code is structured to be extended in future projects

## Future Improvements

Potential enhancements for this project:
- Implementation of pipes between commands (e.g., `cmd1 | cmd2`)
- Job control with foreground/background command management
- Command history functionality
- Tab completion for commands and filenames
- Support for shell scripts and control structures