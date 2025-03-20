# Multi-Precision Integer Calculator

## Overview
This project implements a multi-precision integer calculator in x86 assembly language. It demonstrates low-level programming concepts including struct manipulation, multi-precision arithmetic, dynamic memory allocation, and pseudo-random number generation - all while interfacing with C standard library functions.

The program handles arbitrarily large integers (limited only by available memory) and provides functionality for addition, display, and generation of these large numbers.

## Features

- **Multi-Precision Integer Structure**: Custom implementation of large integers using byte arrays
- **Hexadecimal I/O**: Reading and writing large integers in hexadecimal format
- **Multi-Precision Addition**: Element-wise addition with carry handling
- **Pseudo-Random Number Generation**: Linear Feedback Shift Register (LFSR) implementation
- **Dynamic Memory Allocation**: Assembly-level memory management for result storage
- **Multiple Input Modes**: Command-line arguments for different operational modes

## Building the Project

To compile the project, simply run:

```bash
make
```

The makefile handles the assembly and linking process, producing an executable named `multi`.

## Usage

The program supports three different modes of operation:

### Default Mode
```bash
./multi
```
Adds two predefined multi-precision integers hardcoded in the program.

### Interactive Mode
```bash
./multi -I
```
Reads multi-precision integers from stdin, one per line, performs addition, and displays the result.

### Random Mode
```bash
./multi -R
```
Generates two random multi-precision integers, performs addition, and displays all numbers.

## Technical Implementation

### Multi-Precision Integer Representation
Integers are represented as a struct with the following layout:
```
struct multi {
    unsigned char size;     // Number of "words" in the array
    unsigned char num[];    // Variable-length array containing the integer in little-endian format
}
```

### Addition Algorithm
The addition algorithm handles numbers of different lengths by:
1. Identifying the longer and shorter numbers
2. Adding corresponding bytes with carry propagation
3. Handling any remaining bytes from the longer number
4. Allocating appropriate space for the result

### Pseudo-Random Number Generation
The PRNG uses a 16-bit Linear Feedback Shift Register (LFSR) implementation:
1. Uses a global STATE variable and MASK constant
2. Computes parity of masked bits
3. Shifts STATE with the parity bit feeding into the MSB

## Examples

### Default Mode
```
$ ./multi
4f00440002000100aa
4f004400030002000100aa
4f00930047000400020154
```

### Interactive Mode
```
$ ./multi -I
fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
1
fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
1
100000000000000000000000000000000000000000000000000000000000000000000000
```

### Random Mode
```
$ ./multi -R
5588c47a8814c200cf54a8e1dc23036edd0b196e7d5a8510c1698fdeba56294dd970a69c59249962913d80e3bde487a991057c8a1d15d26e45a6ecb0303c480a19c61360f0eac2c039db49a247b74ac3a0be451ecc8d7f1378196e
7f933c689717b3b90785688ffe379eb82f8faecb679a4bebaa78018761582c968fb622e3bdf4414d5934d765abe8b3597cc218ae337dbaee915580733f71b66af6c0f19a83baa6842885a8f1922439bbeeb1d848e25d2768070d039ee84b539a83aa6060e0b4031e24d9588cd7d52c56e11cd58c8f9e908d27d027304cb1d85824b9ef59ace2b563897c6afe77b4632112508b8db7ca9717a37fe34d5924118163d918b6ca3f79990a194ef092147297b7e22579395b85304ca11ebcec8844b61a1fd454b0189ee074f591c59ac3189e7851e3bd6c645b759d29fdc6ebe2f55994963f319c
7f933c689717b3b90785688ffe379eb82f8faecb679a4bebaa78018761582c968fb622e3bdf4414d5934d765abe8b3597cc218ae337dbaee915580733f71b66af6c0f19a83baa6842885a8f1922439bbeeb1d848e25d2768070d039ee84b539a83aa6060e0b4031e24d9588cd7d52c56e11cd58c8f9e908d27d027304cb1d85824b9ef59ace2b563897cc0873c2eeb35d4515ae260ac733aa6eec05872928edbe8e9da205a1e33ef3367286138b0cbbc5144b6b6ba3f4314d44aafc2691261cbec8e19fb9cc8cf1cbcffab8bae2409893b121d98b606a32ce7ed9e853101c1e713a9b74b0a
```

## Development Notes

- The program is written in NASM syntax for x86 32-bit architecture
- It interfaces with C standard library using the CDECL calling convention
- Special care is taken to manage the stack properly when calling C functions

## Future Improvements

Potential enhancements to this project could include:
- Implementing subtraction, multiplication, and division operations
- Adding decimal display format option
- Implementing algorithms for primality testing of large numbers
- Adding signed integer support
- Implementing comparison operations