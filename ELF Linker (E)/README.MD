# ELF Object File Linker

## Overview
This project implements a custom ELF (Executable and Linkable Format) object file linker for 32-bit systems. It provides capabilities for examining ELF files, displaying their internal structures, and performing limited linking operations to merge multiple object files into a single relocatable object file.

The implementation focuses on the first pass of the linking process, which involves merging sections from multiple input files, resolving symbols, and creating a new ELF file with the combined contents.

## Features

- **ELF File Examination**: Analyze ELF headers, section headers, and symbol tables
- **Section Name Display**: View all sections in an ELF file with their properties
- **Symbol Resolution**: Display symbol information and check for undefined or multiply defined symbols
- **File Merging**: Combine sections from multiple ELF object files into a single relocatable object
- **Memory-Mapped I/O**: Efficiently access file contents using the `mmap` system call

## Building the Project

To compile the project, use the provided makefile:

```bash
make
```

This will produce the executable `myELF`.

## Usage

Run the program with:

```bash
./myELF
```

The program provides an interactive menu with the following options:

```
Choose action:
0-Toggle Debug Mode
1-Examine ELF File
2-Print Section Names
3-Print Symbols
4-Check Files for Merge
5-Merge ELF Files
6-Quit
```

### Workflow

1. Use **Examine ELF File** to open up to two object files
2. Use **Print Section Names** and **Print Symbols** to analyze the files
3. Use **Check Files for Merge** to verify that the files can be successfully merged
4. Use **Merge ELF Files** to create a new `out.ro` file containing the merged content

## Implementation Details

### ELF File Examination
The program maps the entire ELF file into memory using `mmap` and analyzes its header structures to extract:
- Magic number validation
- Data encoding scheme
- Entry point address
- Section header table information
- Program header table information

### Section Analysis
For each section in the ELF file, the program displays:
- Index
- Name (extracted from the `.shstrtab` section)
- Memory address
- File offset
- Size in bytes
- Section type

### Symbol Table Parsing
The program extracts symbol information including:
- Symbol index
- Value
- Section index where the symbol is defined
- Section name
- Symbol name (extracted from the `.strtab` section)

### Merge Process
The merging operation:
1. Creates a new ELF file with a header based on the first input file
2. Creates a new section header table based on the first input file
3. Merges sections (`.text`, `.data`, `.rodata`) by concatenating their contents
4. Copies special sections (`.shstrtab`, `.symtab`) directly
5. Updates section offsets and sizes in the section header table
6. Updates the ELF header to reflect the new section header table offset

## Technical Requirements

- 32-bit Linux environment (or 64-bit with 32-bit compatibility)
- GCC compiler with 32-bit support
- Knowledge of ELF file format and linking process

## Limitations

This implementation has the following limitations:
- Supports only 32-bit ELF files
- Can merge only two ELF files at a time
- The second ELF file cannot contain sections absent in the first file
- The second ELF file cannot contain symbols absent in the first file
- Relocation processing is simplified

## Future Improvements

Potential enhancements to this project could include:
- Support for 64-bit ELF files
- Handling of more than two input files
- Support for sections and symbols present only in the second file
- Full relocation processing
- Symbol table merging with proper resolution of undefined symbols

## References

- [ELF Format Specification](https://refspecs.linuxfoundation.org/elf/gabi4+/contents.html)
- [mmap(2) - Linux manual page](https://man7.org/linux/man-pages/man2/mmap.2.html)
- [readelf(1) - Linux manual page](https://man7.org/linux/man-pages/man1/readelf.1.html)
- [elf(5) - Linux manual page](https://man7.org/linux/man-pages/man5/elf.5.html)