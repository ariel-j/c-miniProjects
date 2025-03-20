# ELF Static Loader

## Overview
This project implements a static loader for ELF (Executable and Linkable Format) binaries on Linux systems. The loader can directly execute 32-bit ELF executables without requiring the operating system's standard loader, providing insights into how operating systems load and execute programs.

The implementation focuses on understanding the internal structure of ELF files, particularly program headers, and using memory mapping techniques to load executable segments into the appropriate memory locations.

## Features
- **Program Header Iteration**: Traverses through all program headers in an ELF file
- **Header Analysis**: Displays detailed information about each program header similar to `readelf -l`
- **Memory Mapping**: Maps program segments to memory with appropriate permissions
- **Execution Control**: Passes control to loaded executables with command-line arguments
- **Static Binary Support**: Loads and executes statically linked executables (no dynamic library dependencies)

## Building the Project
To compile the project, use the provided makefile:

```bash
make
```

Note: This project requires special linking to avoid memory conflicts. It uses a custom linking script to ensure the loader doesn't occupy the same memory space as the programs it loads.

## Usage
Run the loader with:

```bash
./loader <executable_path> [arg1 arg2 ...]
```

Where:
- `<executable_path>` is the path to a 32-bit ELF executable you want to load
- `[arg1 arg2 ...]` are optional command-line arguments to pass to the loaded program

### Example
```bash
./loader ./my_program hello world
```

This loads `my_program` and passes it the arguments "hello" and "world".

## Implementation Details

### Program Header Iterator
The core of the loader is an iterator function that applies a callback to each program header:

```c
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg);
```

This provides a flexible way to process all program headers in an ELF file.

### Loading Process
The loading process involves:

1. Opening and memory-mapping the target ELF file
2. Analyzing the ELF header to find the program headers
3. Iterating through program headers and mapping segments with PT_LOAD flag
4. Setting appropriate memory protections based on segment flags
5. Jumping to the entry point and passing control to the loaded program

### Custom Linking
To avoid memory space conflicts, the loader is compiled with a custom linking script that places the loader's code in a memory region that won't interfere with the loaded program.

## Technical Requirements
- Linux environment (32-bit or 64-bit capable of running 32-bit code)
- GCC compiler with 32-bit support
- Basic understanding of ELF format and memory mapping concepts

## Limitations
- Only supports 32-bit ELF executables
- Does not support dynamic linking (no shared library dependencies)
- Loaded programs must use system calls directly (no libc dependency)
- Limited error handling for malformed ELF files

## Future Improvements
- Support for 64-bit ELF executables
- Support for dynamically linked executables
- Enhanced error handling and validation
- Memory cleanup after program execution
- Support for additional ELF features like TLS (Thread Local Storage)

## References
- [ELF Format Specification](https://refspecs.linuxfoundation.org/elf/gabi4+/contents.html)
- [mmap(2) - Linux manual page](https://man7.org/linux/man-pages/man2/mmap.2.html)
- [readelf(1) - Linux manual page](https://man7.org/linux/man-pages/man1/readelf.1.html)