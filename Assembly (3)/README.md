# Assembly Language Lab 3 - System Calls and Low-Level Programming

## Overview
This repository demonstrates my skills in assembly language programming and low-level system interactions through direct system calls. The lab focuses on fundamental assembly programming, system calls, and integrating assembly with C programs.

## Lab Objectives
1. Learn the basics of assembly language programming.
2. Use NASM assembler and GCC linker to create executables without standard libraries.
3. Implement system calls directly for I/O operations and process control.
4. Develop low-level encoding and decoding programs.
5. Manipulate files and directories without relying on high-level libraries.
6. Attach executable code to files as part of virus-like behavior simulations.

## Tasks and Implementation

### Task 0: Assembly Language Primer
**Goal:** Create a stand-alone program in assembly to print 'Hello, World!' using system calls.
- **File:** `hello_world.s`
- **Commands:**
  ```bash
  nasm -f elf32 hello_world.s -o hello_world.o
  ld -m elf_i386 hello_world.o -o hello_world
  ./hello_world
  ```
- **Description:** Uses the `sys_write` and `sys_exit` system calls to output a string and exit gracefully.

### Task 1: Simplified Encoder
**Goal:** Implement a simplified character encoder in assembly language.
- **Subtasks:**
  - **Debug Printout (Task 1.A):** Print command-line arguments directly using `sys_write`.
  - **Basic Encoder (Task 1.B):** Encode input from stdin and output to stdout.
  - **Encoder with I/O Support (Task 1.C):** Accept input and output files via command-line arguments and process data.
- **Files:**
  - `task1/start.s` - Assembly implementation.
  - `task1/makefile` - Automates the build process.

### Task 2: File Manipulation and Virus Simulation
**Goal:** List files in a directory and attach code to executables.
- **Subtasks:**
  - **Directory Listing (Task 2.A):** Use the `sys_getdents` system call to list files in the current directory.
  - **Virus Attachment (Task 2.B):** Append executable code to files matching a specified prefix.
- **Files:**
  - `task2/main.c` - C implementation for directory listing.
  - `task2/start.s` - Assembly implementation for file manipulation.
  - `task2/makefile` - Automates the build process.

## Key Features
- **No Standard Library Usage:** All tasks avoid using standard libraries such as `stdio.h` and rely on direct system calls.
- **Modular Design:** C and assembly code are linked seamlessly.
- **Makefiles for Automation:** Simplifies the build process with defined rules.
- **Low-Level File Operations:** Demonstrates reading, writing, and modifying files.
- **Custom Encoders:** Encodes data by manipulating character values directly.
- **Simulated Virus Behavior:** Demonstrates the mechanics of appending executable code to files.

## Dependencies
- **NASM:** Assembler for writing low-level code.
- **GCC:** Compiler for integrating assembly with C.
- **Linux OS:** Required for executing system calls.

## Building and Running Programs
### Task 0 Example:
```bash
cd task0
nasm -f elf32 hello_world.s -o hello_world.o
ld -m elf_i386 hello_world.o -o hello_world
./hello_world
```
### Task 1 Example:
```bash
cd task1
make
./task1 -iinput.txt -ooutput.txt
```
### Task 2 Example:
```bash
cd task2
make
./task2 -aP
```

## Notes
- Ensure executable permissions are set:
  ```bash
  chmod +x task0/hello_world task1/task1 task2/task2
  ```
- Test the virus attachment task with caution to avoid corrupting important files.

## Contributions
This lab is designed to solidify understanding of low-level programming concepts. If you have suggestions or improvements, feel free to submit a pull request.

## License
This project is licensed under the MIT License.

