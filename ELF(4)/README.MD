# HexEditPlus - ELF Binary Manipulation Tool

## Overview
HexEditPlus is a powerful binary manipulation tool designed to read, analyze, and modify ELF (Executable and Linkable Format) files. It provides a simple yet comprehensive interface for various operations such as loading file contents into memory, displaying file and memory contents, saving modified data back to files, and directly modifying memory.

This project was developed as part of my exploration into systems programming and binary analysis while finishing my Computer Science degree.

## Features
- **Flexible Unit Size**: Manipulate data in units of 1, 2, or 4 bytes
- **Debug Mode**: Toggle detailed debug information for development and learning
- **File Operations**: Load, display, and save binary data with flexible offset selection
- **Memory Operations**: Display and modify in-memory data with decimal or hexadecimal representation
- **ELF Analysis**: Specialized for working with ELF binaries, allowing inspection and modification of section headers, entry points, and function code

## Building the Project
To compile the project, simply run:

```
make
```

The makefile will produce the executable `hexeditplus`.

## Usage
Run the program with:

```
./hexeditplus
```

The program will present a menu with the following options:

```
Choose action:
0-Toggle Debug Mode
1-Set File Name
2-Set Unit Size
3-Load Into Memory
4-Toggle Display Mode
5-File Display
6-Memory Display
7-Save Into File
8-Memory Modify
9-Quit
```

### Basic Workflow
1. Set the file name using option 1
2. Set the unit size (1, 2, or 4 bytes) using option 2
3. Load data from the file into memory using option 3
4. View or modify the data using options 5, 6, and 8
5. Save the modified data back to a file using option 7

### Examples

#### Loading Data into Memory
```
3
Please enter <location> <length>
12F 10
```
This loads 10 units (based on current unit size) from file offset 0x12F.

#### Displaying File Contents
```
5
Enter file offset and length
12F 5
```
This displays 5 units starting from file offset 0x12F.

#### Modifying Memory
```
8
Please enter <location> <val>
40 804808a
```
This modifies the memory at offset 0x40 with the value 0x804808a.

## Advanced Applications

HexEditPlus was designed with the following capabilities in mind:

### Binary Patching
The tool can be used to patch binary files, such as fixing executables with incorrect entry points or modifying specific function code.

### Reverse Engineering
By examining ELF structures and section content, HexEditPlus helps understand the structure and behavior of compiled programs.

### Software Security
The tool can be used to study software security by examining how modifications to binary files affect program execution.

## Implementation Notes
- The program handles data in user-defined unit sizes for all operations
- Memory operations work on an internal buffer with a size of 10,000 bytes
- The tool implements careful bounds checking on all memory and file operations

## Future Improvements
I'm considering adding the following features in future versions:
- Support for 64-bit ELF files
- Automatic ELF header and section table parsing
- Symbol table navigation
- Disassembly integration

## License
This project is provided for educational purposes and personal use.