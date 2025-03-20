# Virus Detector Project

## Overview
This project is a lab assignment in C programming that focuses on debugging, dynamic data structures (linked lists), and patching binary files. The assignment introduces concepts such as memory management, virus detection, and basic file manipulation.

### Assignment Goals:
- Implementing and using **linked lists** in C.
- Debugging memory issues using **Valgrind**.
- Manipulating binary files.
- Understanding virus detection through naive algorithms.

The project includes several tasks, culminating in a fully functional antivirus simulation.

## Features
1. **Linked List Implementation**
   - Dynamically manage virus signature data.
   - Traverse, append, and free memory associated with linked lists.
2. **Hexadecimal Printout**
   - Display binary file contents in hexadecimal format.
3. **Virus Detection**
   - Detect viruses by comparing file contents to stored signatures.
4. **Virus Neutralization**
   - Automatically modify infected files to neutralize viruses.
5. **Debugging with Valgrind**
   - Identify and fix memory leaks and invalid memory accesses.

## Usage

### Prerequisites
1. Install Valgrind:
   ```bash
   sudo apt-get install valgrind
   ```
2. Install debugging libraries (if on a virtual machine):
   ```bash
   sudo apt-get install libc6-dbg:i386
   ```

### Compilation
Use the provided `Makefile` to compile the project:
```bash
make
```
This will generate an executable named `virusDetector`.

### Running the Program
Execute the program:
```bash
./virusDetector
```
The program provides a menu with the following options:
1. Load Signatures
2. Print Signatures
3. Detect Viruses
4. Fix File
5. Quit

### Menu Options Explained
1. **Load Signatures**:
   - Loads virus signatures from a binary file into memory and stores them in a linked list.
   - The binary file must follow the specified format (see Virus Signature Format below).
2. **Print Signatures**:
   - Displays the loaded virus signatures in human-readable format.
3. **Detect Viruses**:
   - Prompts for a file to scan and checks it against loaded virus signatures.
4. **Fix File**:
   - Neutralizes detected viruses by modifying their signatures in the file.
5. **Quit**:
   - Exits the program and releases all allocated memory.

## Virus Signature Format
Each virus signature in the file has the following structure:
- **Magic Number** (4 bytes): Identifies the file format ("VIRL" for little-endian or "VIRB" for big-endian).
- **Virus Details**:
  - **Length** (2 bytes): Length of the signature.
  - **Name** (16 bytes): Null-terminated virus name.
  - **Signature**: Actual virus signature data.

## Key Functions
### File Manipulation
- **readVirus(FILE *f)**: Reads and parses a single virus record from the file.
- **printVirus(virus *v, FILE *output)**: Prints virus details in human-readable format.

### Linked List Operations
- **list_append(link *list, virus *data)**: Adds a new virus to the linked list.
- **list_print(link *list, FILE *output)**: Prints all viruses in the linked list.
- **list_free(link *list)**: Frees all memory used by the linked list.

### Virus Detection and Neutralization
- **detect_virus(char *buffer, unsigned int size, link *virus_list)**: Scans a file buffer for virus signatures.
- **neutralize_virus(char *fileName, int signatureOffset)**: Replaces the virus signature with a `RET` instruction.

## Debugging Tips
- Use **Valgrind** to ensure memory safety:
  ```bash
  valgrind --leak-check=full ./virusDetector
  ```
- Use **gdb** to debug segmentation faults:
  ```bash
  gdb ./virusDetector
  ```

## Example Usage
### Hexadecimal Printout
To print the hexadecimal contents of a file:
```bash
./hexaPrint exampleFile
```
Output:
```
63 68 65 63 6B AA DD 4D 79 0C 48 65 78
```

### Virus Detection
Load signatures and scan a file:
```bash
1
Enter signature file name: signatures.bin
2
3
Enter suspected file name: infected_file.bin
```

### Neutralization
Fix an infected file:
```bash
4
Enter file name to fix: infected_file.bin
```

Test the archive by extracting and running the program before submission.

