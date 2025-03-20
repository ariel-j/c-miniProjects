# C Memory Management and Function Pointers

## Overview
This project explores fundamental concepts of memory management, pointers, and function manipulation in C. It demonstrates how to work with memory addresses, implement function pointers, and simulate object-oriented behavior in a procedural language.

The implementation consists of three main components:
1. Memory address analysis and pointer manipulation
2. Function pointer implementation with character mapping
3. An interactive menu system using function pointers to simulate objects

## Features

### Memory Analysis Tools
- Visualization of memory layout for different data types
- Address spacing analysis for stack, heap, and code segments
- Array memory representation and pointer arithmetic demonstration
- Command-line argument memory location analysis

### Function Pointer Utilities
- Character mapping system with function pointer callbacks
- Various character transformation functions:
  - Character encryption/decryption
  - Decimal and hexadecimal representation
  - Character printing with validation
  - Input character retrieval

### Interactive Menu System
- Menu implementation using structures with function pointers
- Dynamic character array manipulation
- Support for various text transformations via mapped functions
- Clean memory management with proper allocation/deallocation

## Building the Project

The project can be built using the provided makefile:

```bash
make
```

This will compile the `menu` executable, which contains the complete implementation of all features.

## Usage

Run the interactive menu system:

```bash
./menu
```

The program will display a numbered menu of available operations. Enter the number corresponding to the desired operation:

```
Select operation from the following menu:
1) Get String
2) Print Decimal
3) Print Hex
4) Print Character
5) Encrypt
6) Decrypt
0) Exit

Enter operation:
```

### Example Operations

1. **Get String**: Reads a string from standard input
2. **Print Decimal**: Displays the decimal value of each character
3. **Print Hex**: Displays the hexadecimal value of each character
4. **Print Character**: Prints each character (or '.' for non-printable characters)
5. **Encrypt**: Adds 1 to the ASCII value of each character
6. **Decrypt**: Subtracts 1 from the ASCII value of each character

### Sample Workflow

```
Select operation from the following menu:
1) Get String
2) Print Decimal
3) Print Hex
4) Print Character
5) Encrypt
6) Decrypt
0) Exit

Enter operation: 1
Enter string: Hello

Select operation from the following menu:
...

Enter operation: 4
H
e
l
l
o

Select operation from the following menu:
...

Enter operation: 5
[encryption performed]

Select operation from the following menu:
...

Enter operation: 4
I
f
m
m
p
```

## Implementation Details

### Memory Mapping Function
The core `map` function takes an array of characters, applies a specified function to each character, and returns a new array:

```c
char* map(char *array, int array_length, char (*f) (char))
```

### Character Processing Functions
- `my_get`: Reads a character from stdin
- `cprt`: Prints a character if it's printable
- `encrypt`: Adds 1 to a character's ASCII value
- `decrypt`: Subtracts 1 from a character's ASCII value
- `xprt`: Prints a character in hexadecimal
- `dprt`: Prints a character in decimal

### Function Descriptor Structure
The menu system uses a structure to hold function names and pointers:

```c
struct fun_desc {
    char *name;
    char (*fun)(char);
};
```

## Technical Notes

- The program is designed for 32-bit systems (compiled with -m32)
- All memory allocations are properly managed to prevent leaks
- Function pointers demonstrate how to simulate object-oriented methods in C
- The menu system uses an array of function descriptors terminated with NULL values

## Future Improvements

- Support for longer strings and dynamic array resizing
- Additional character transformation functions
- File I/O capability for processing text files
- Command-line arguments for batch processing