#include <elf.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

// ======================================== PART 0 ========================================

/**
 * Write a program, which gets a single command line argument. The argument will be the file name of a 32bit ELF formatted executable.
 *
 * Your task is to write an iterator over program headers in the file. Implement a function with the following signature:
 * int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
 * 
 * @map_start:The address in virtual memory the executable is mapped to.
 * @func:The function which will be applied to each Phdr.
 * @arg: An additional argument to be passed to func, for later use (not used in this task - taks0).
 * 
 * This function will apply func to each program header.
 * Verify that your iterator works by applying it to an 32bit ELF file,
 * with a function that prints out a message: 
 * Program header number i at address x" for each program header i it visits.
 * 
 * @return: 0 for succes
 */
// int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
//     Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
//     Elf32_Phdr *phdr = (Elf32_Phdr *)((char *)map_start + ehdr->e_phoff);

//     printf("Type    |    Offset    |    VirtAddr    |    PhysAddr    |    FileSiz    |    MemSiz     |    Flg    |    Align\n"); //part 1A
    
//     for (int i = 0; i < ehdr->e_phnum; i++) {
//         func(&phdr[i], i);
//     }
    
//     return 0;
// }

// void print_phdr(Elf32_Phdr *phdr, int index) {
//     printf("Program header number %d at address %p\n", index, (void*)phdr);
//     printf("  Offset: 0x%x\n", phdr->p_offset);  // Should print 0x34 for the first header if we run it on our executable for part0. (readelf -l part0)
// }

// ======================================== PART 1A ========================================

void print_header_type(Elf32_Word type) {
    switch(type) {
        case PT_NULL: printf("NULL    |  "); break;
        case PT_LOAD: printf("LOAD    |  "); break;
        case PT_DYNAMIC: printf("DYNAMIC |  "); break;
        case PT_INTERP: printf("INTERP  |  "); break;
        case PT_NOTE: printf("NOTE    |  "); break;
        case PT_PHDR: printf("PHDR    |  "); break;
        default: printf("UNKNOWN |  "); break;
    }
}

void print_flags(Elf32_Word flags) {
    printf("   %c%c%c    |", 
        (flags & PF_R) ? 'R' : ' ',         // Read
        (flags & PF_W) ? 'W' : ' ',         // Write
        (flags & PF_X) ? 'E' : ' '          // Execute
    );
}

// void print_phdr(Elf32_Phdr *phdr, int index) {
//     print_header_type(phdr->p_type);
//     printf(" ");
//     printf("0x%06x   |  ", phdr->p_offset);      // Offset
//     printf("0x%08x    |  ", phdr->p_vaddr);       // VirtAddr
//     printf("0x%08x    | ", phdr->p_paddr);       // PhysAddr
//     printf("   0x%05x    | ", phdr->p_filesz);      // FileSiz
//     printf("   0x%05x    | ", phdr->p_memsz);       // MemSiz
    
//     print_flags(phdr->p_flags);             // Flags
//     printf("    0x%x\n", phdr->p_align);       // Align
// }

// ======================================== PART 1B ========================================

// int get_prot_flags(Elf32_Word flags) {
//     int prot = PROT_NONE;
//     // |= bitwise OR operator
//     if (flags & PF_R) prot |= PROT_READ;
//     if (flags & PF_W) prot |= PROT_WRITE;
//     if (flags & PF_X) prot |= PROT_EXEC;
//     return prot;
// }

void print_phdr(Elf32_Phdr *phdr, int index) {
    print_header_type(phdr->p_type);
    printf(" ");
    printf("0x%06x   |  ", phdr->p_offset);      // Offset
    printf("0x%08x    |  ", phdr->p_vaddr);       // VirtAddr
    printf("0x%08x    | ", phdr->p_paddr);       // PhysAddr
    printf("   0x%05x    | ", phdr->p_filesz);      // FileSiz
    printf("   0x%05x    | ", phdr->p_memsz);       // MemSiz
    
    print_flags(phdr->p_flags);             // Flags
    printf("    0x%x\n", phdr->p_align);       // Align

    // if (phdr->p_type == PT_LOAD) {
    //     int prot = get_prot_flags(phdr->p_flags);
    //     printf("   Protection flags for mmap: ");
    //     if (prot == PROT_NONE) printf("PROT_NONE");
    //     else {
    //         if (prot & PROT_READ) printf("PROT_READ ");
    //         if (prot & PROT_WRITE) printf("| PROT_WRITE ");
    //         if (prot & PROT_EXEC) printf("| PROT_EXEC");
    //     }
    //     printf("\n");
    //     printf("   Mapping flags for mmap: MAP_PRIVATE  | MAP_FIXED\n");
    //     printf("   (Required alignment: %d)\n\n", phdr->p_align);
    // }
}

// ======================================== PART 2B ========================================

/***
 *  the function maps each Phdr that has the PT_LOAD flag set,
 *  into memory, starting from the specified offset,
 *  and place it at the virtual address stated in the Phdr.
 *  Each map should be according to the flags set in the Phdr struct.
 *  In addition, this function should print to the screen
 *  the information about each program header it maps 
 * (you can use the function from Task 1 to print the information).
 * 
 * @phdr: a pointer to the Phdr struct.
 * @fd: file descriptor of the executable file.
 * Recommended operating procedure: make sure system calls succeed before proceeding, most especially mmap.
 */
void load_phdr(Elf32_Phdr *phdr, int fd) {
    if(phdr->p_type != PT_LOAD) return;

    int prot = PROT_NONE;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    Elf32_Addr vaddr = phdr->p_vaddr & 0xfffff000;
    Elf32_Off offset = phdr->p_offset & 0xfffff000;
    size_t padding = phdr->p_vaddr & 0xfff;

    void *map_start = mmap(
        (void *)vaddr,
        phdr->p_memsz + padding,
        prot,
        MAP_PRIVATE | MAP_FIXED,
        fd,
        offset
    );

    if (map_start == MAP_FAILED) {
        perror("Error mapping segment");
        exit(1);
    }
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)((char *)map_start + ehdr->e_phoff);

    if (func == print_phdr) {
        printf("Type    |    Offset    |    VirtAddr    |    PhysAddr    |    FileSiz    |    MemSiz     |    Flg    |    Align\n");
    }
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        func(&phdr[i], arg);
    }
    
    return 0;
}
// ======================================== PART 2C ========================================

extern int startup(int argc, char** argv, void (*start)());

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <executable> [args...]\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    void *map_start = mmap(NULL, file_size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return 1;
    }

    // Load segments first
    foreach_phdr(map_start, load_phdr, fd);
    
    // Then print info
    foreach_phdr(map_start, print_phdr, 0);

    // Get entry point and start program
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    void (*entry_point)() = (void (*)())ehdr->e_entry;

    printf("\nTransferring control to program at address 0x%x\n", (unsigned int)entry_point);
    printf("Arguments being passed: %d\n", argc-1);
    for(int i = 1; i < argc; i++) {
        printf("arg[%d]: %s\n", i-1, argv[i]);
    }

    startup(argc-1, argv+1, entry_point);

    munmap(map_start, file_size);
    close(fd);
    
    return 0;
}