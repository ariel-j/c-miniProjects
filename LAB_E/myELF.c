#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>

// ============================== Constants ==============================

#define MAX_INPUT_SIZE 100
#define MAX_BUFFER_SIZE 10000
#define MAX_FILENAME_SIZE 128
#define MIN_UNIT_SIZE 1
#define MAX_UNIT_SIZE 4
#define BUFFER_SIZE 256
#define MAX_FILES 2
// ============================== Structs ==============================

typedef struct {
    char debug_mode;
    char file_name[MAX_FILENAME_SIZE];
    int unit_size;
    unsigned char mem_buf[MAX_BUFFER_SIZE];
    size_t mem_count;
    char display_mode;
} state;

typedef struct fun_desc{
    char *name;
    void (*func)();
} fun_desc;

typedef struct {
    int fd;
    void *map_start;
    size_t file_size;
    Elf32_Ehdr *elf_header;
} ElfFile;

// ============================ Global variables ===================================
static char debug_mode = 0;
int file_count = 0;
ElfFile elf_files[MAX_FILES];

// ============================== Utility Functions ==============================


int validation(int len, char input []) {
    if (len == 0 || len >= 5 - 1) return 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] < '0' || input[i] > '9') {
            return 0;
        }
    }
    return 1;
}

// ============================== Debug Functions ==============================
void print_debug_info(const state* s) {
    fprintf(stderr, "Debug Information:\n");
    fprintf(stderr, "Unit Size: %d\n", s->unit_size);
    fprintf(stderr, "File Name: %s\n", s->file_name);
    fprintf(stderr, "Memory Count: %zu\n", s->mem_count);
}

// Toggles the debug mode, printing the current state.
void toggle_debug_mode() {
    debug_mode = !debug_mode;
    printf("Debug mode %s\n", debug_mode ? "ON" : "OFF");
}

// ============================= Menu Functions ==============================

// ============================= 0 =============================

void examine_elf_file() {

    if (file_count >= MAX_FILES) {
        printf("Cannot open more than %d ELF files\n", MAX_FILES);
        return;
    }

    char file_name[BUFFER_SIZE];
    printf("Enter ELF file name: ");
    fgets(file_name, BUFFER_SIZE, stdin);
    file_name[strcspn(file_name, "\n")] = 0;  // Remove trailing newline.

    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return;
    }

    size_t file_size = lseek(fd, 0, SEEK_END);  // Get the file size.
    lseek(fd, 0, SEEK_SET);  // Reset the file offset.

    void *map_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);  // Map the file into memory.
    if (map_start == MAP_FAILED) {
        perror("Failed to mmap file");
        close(fd);
        return;
    }

    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;  // Cast the mapped memory to an ELF header structure.

    // Verify the ELF magic number to confirm it's a valid ELF file.
    if (memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Not a valid ELF file\n");
        munmap(map_start, file_size);
        close(fd);
        return;
    }

    // Store the ELF file information.
    elf_files[file_count].fd = fd;
    elf_files[file_count].map_start = map_start;
    elf_files[file_count].file_size = file_size;
    elf_files[file_count].elf_header = elf_header;
    file_count++;

    // Print ELF header details.
    printf("Magic: %c%c%c\n", elf_header->e_ident[EI_MAG1], elf_header->e_ident[EI_MAG2], elf_header->e_ident[EI_MAG3]);
    printf("Data: %s\n", elf_header->e_ident[EI_DATA] == ELFDATA2LSB ? "Little Endian" : "Big Endian");
    printf("Entry point: 0x%x\n", elf_header->e_entry);
    printf("Section header table offset: 0x%x\n", elf_header->e_shoff);
    printf("Number of section headers: %d\n", elf_header->e_shnum);
    printf("Size of section headers: %d\n", elf_header->e_shentsize);
    printf("Program header table offset: 0x%x\n", elf_header->e_phoff);
    printf("Number of program headers: %d\n", elf_header->e_phnum);
    printf("Size of program headers: %d\n", elf_header->e_phentsize);
}

// ============================= 1 =============================

void print_section_names() {
    if (file_count == 0) {
        printf("No ELF files loaded\n");
        return;
    }

    for (int file_idx = 0; file_idx < file_count; file_idx++) {
        ElfFile *current_file = &elf_files[file_idx];
        Elf32_Ehdr *header = current_file->elf_header;
        
        // Get section header table
        Elf32_Shdr *section_headers = (Elf32_Shdr *)((char *)current_file->map_start + header->e_shoff);
        
        // Debug print for shstrndx before processing
        if (debug_mode) {
            fprintf(stderr, "\nDebug Info for File %d:\n", file_idx + 1);
            fprintf(stderr, "Section header string table index (shstrndx): %d\n", header->e_shstrndx);
            fprintf(stderr, "Section header table offset: 0x%x\n", header->e_shoff);
        }

        // Get string table section header
        Elf32_Shdr *str_tab = &section_headers[header->e_shstrndx];
        
        if (debug_mode) {
            fprintf(stderr, "String table offset: 0x%x\n", str_tab->sh_offset);
            fprintf(stderr, "String table size: %d bytes\n", str_tab->sh_size);
        }

        // Get actual string table
        const char *str_tab_start = (const char *)current_file->map_start + str_tab->sh_offset;

        printf("\nFile %d:\n", file_idx + 1);
        printf("[Nr] %-17s %-10s %-10s %-10s %s\n", 
               "Name", "Addr", "Off", "Size", "Type");

        for (int i = 0; i < header->e_shnum; i++) {
            Elf32_Shdr *section = &section_headers[i];
            const char *name = str_tab_start + section->sh_name;

            if (debug_mode) {
                fprintf(stderr, "\nDebug: Section %d:\n", i);
                fprintf(stderr, "  Name offset in string table: %d\n", section->sh_name);
                fprintf(stderr, "  Section offset in file: 0x%x\n", section->sh_offset);
                fprintf(stderr, "  Section size: %d bytes\n", section->sh_size);
            }

            printf("[%2d] %-17s %08x  %08x  %08x  %d\n", 
                   i,
                   name,
                   section->sh_addr,
                   section->sh_offset,
                   section->sh_size,
                   section->sh_type
            );
        }
    }
}

// ============================= 2 =============================
// Find symbol table section
Elf32_Shdr* find_symbol_table(Elf32_Shdr* section_headers, int num_sections) {
    for (int i = 0; i < num_sections; i++) {
        if (section_headers[i].sh_type == SHT_SYMTAB) {
            return &section_headers[i];
        }
    }
    return NULL;
}

// Get section name, handling special cases
const char* get_section_name(Elf32_Half st_shndx, Elf32_Half num_sections, 
                           const char* shstrtab_data, Elf32_Shdr* section_headers) {
    if (st_shndx == SHN_UNDEF || 
        st_shndx >= num_sections || 
        st_shndx == SHN_ABS ||
        st_shndx == SHN_COMMON) {
        return "UND";
    }
    return shstrtab_data + section_headers[st_shndx].sh_name;
}

// Print debug information
void print_symbol_debug_info(int file_idx, Elf32_Shdr* symtab) {
    fprintf(stderr, "\nDebug Info for File %d:\n", file_idx + 1);
    fprintf(stderr, "Symbol table size: %d bytes\n", symtab->sh_size);
    fprintf(stderr, "Number of symbols: %d\n", symtab->sh_size / sizeof(Elf32_Sym));
    fprintf(stderr, "Symbol table offset: 0x%x\n", symtab->sh_offset);
}

// Print a single symbol entry
void print_symbol_entry(int index, Elf32_Sym* sym, const char* symbol_name, const char* section_name) {
    printf("[%2d] %08x %3d %-16s %s\n",
           index,
           sym->st_value,
           sym->st_shndx,
           section_name,
           symbol_name[0] ? symbol_name : "");
}

// Process symbols for a single file
void process_file_symbols(ElfFile* current_file, int file_idx) {
    Elf32_Ehdr* header = current_file->elf_header;
    Elf32_Shdr* section_headers = (Elf32_Shdr*)((char*)current_file->map_start + header->e_shoff);
    
    // Find symbol table
    Elf32_Shdr* symtab = find_symbol_table(section_headers, header->e_shnum);
    if (!symtab) {
        printf("File %d: No symbol table found\n", file_idx + 1);
        return;
    }

    // Get string tables
    Elf32_Shdr* strtab = &section_headers[symtab->sh_link];
    if (strtab->sh_type != SHT_STRTAB) {
        printf("File %d: Invalid string table\n", file_idx + 1);
        return;
    }

    Elf32_Shdr* shstrtab = &section_headers[header->e_shstrndx];
    const char* shstrtab_data = (char*)current_file->map_start + shstrtab->sh_offset;
    const char* strtab_data = (char*)current_file->map_start + strtab->sh_offset;
    
    // Get symbol table entries
    Elf32_Sym* symbols = (Elf32_Sym*)((char*)current_file->map_start + symtab->sh_offset);
    int symbol_count = symtab->sh_size / sizeof(Elf32_Sym);

    if (debug_mode) {
        print_symbol_debug_info(file_idx, symtab);
    }

    printf("\nFile %d:\n", file_idx + 1);

    for (int i = 0; i < symbol_count; i++) {
        Elf32_Sym* sym = &symbols[i];
        const char* symbol_name = sym->st_name ? strtab_data + sym->st_name : "";
        const char* section_name = get_section_name(sym->st_shndx, header->e_shnum, 
                                                  shstrtab_data, section_headers);
        
        print_symbol_entry(i, sym, symbol_name, section_name);
    }
}

// Main print symbols function
void print_symbols() {
    if (file_count == 0) {
        printf("No ELF files loaded\n");
        return;
    }

    for (int file_idx = 0; file_idx < file_count; file_idx++) {
        process_file_symbols(&elf_files[file_idx], file_idx);
    }
}


//==================================== 3.1 ==============================

// Helper function to check if a symbol is global
int is_symbol_global(Elf32_Sym* sym) {
    return ELF32_ST_BIND(sym->st_info) == STB_GLOBAL;
}

// Validates that we have exactly 2 files with 1 symbol table each
int validate_merge_requirements() {
    if (file_count != 2) {
        printf("Error: Must have exactly 2 files\n");
        return 0;
    }

    // Check symbol table count for each file
    for (int i = 0; i < 2; i++) {
        ElfFile* curr_file = &elf_files[i];
        Elf32_Shdr* section_headers = (Elf32_Shdr*)((char*)curr_file->map_start + 
                                                   curr_file->elf_header->e_shoff);
        
        int symtab_count = 0;
        for (int j = 0; j < curr_file->elf_header->e_shnum; j++) {
            if (section_headers[j].sh_type == SHT_SYMTAB) {
                symtab_count++;
            }
        }
        
        if (symtab_count != 1) {
            printf("Feature not supported\n");
            return 0;
        }
    }
    return 1;
}

// Finds the symbol table section in an ELF file
Elf32_Shdr* find_symbol_table_section(ElfFile* elf_file) {
    Elf32_Shdr* sections = (Elf32_Shdr*)((char*)elf_file->map_start + 
                                        elf_file->elf_header->e_shoff);
    
    for (int i = 0; i < elf_file->elf_header->e_shnum; i++) {
        if (sections[i].sh_type == SHT_SYMTAB) {
            return &sections[i];
        }
    }
    return NULL;
}

// Finds a symbol in a symbol table (only global symbols)
Elf32_Sym* find_symbol_in_table(const char* symbol_name, Elf32_Sym* symtab, 
                               int sym_count, const char* strtab) {
    for (int i = 1; i < sym_count; i++) {
        if (!is_symbol_global(&symtab[i])) continue;
        const char* curr_name = strtab + symtab[i].st_name;
        if (strcmp(symbol_name, curr_name) == 0) {
            return &symtab[i];
        }
    }
    return NULL;
}

// Checks for symbol conflicts between two files
void check_symbol_conflicts(ElfFile* curr_file, ElfFile* other_file, int is_first_file) {
    // Get current file's symbol table and string table
    Elf32_Shdr* curr_sections = (Elf32_Shdr*)((char*)curr_file->map_start + 
                                             curr_file->elf_header->e_shoff);
    Elf32_Shdr* curr_symtab_section = find_symbol_table_section(curr_file);
    if (!curr_symtab_section) return;

    Elf32_Shdr* curr_strtab = &curr_sections[curr_symtab_section->sh_link];
    const char* curr_strtab_data = (char*)curr_file->map_start + curr_strtab->sh_offset;
    Elf32_Sym* curr_symbols = (Elf32_Sym*)((char*)curr_file->map_start + 
                                          curr_symtab_section->sh_offset);
    int curr_sym_count = curr_symtab_section->sh_size / sizeof(Elf32_Sym);

    // Get other file's symbol table and string table
    Elf32_Shdr* other_sections = (Elf32_Shdr*)((char*)other_file->map_start + 
                                              other_file->elf_header->e_shoff);
    Elf32_Shdr* other_symtab_section = find_symbol_table_section(other_file);
    if (!other_symtab_section) return;

    Elf32_Shdr* other_strtab = &other_sections[other_symtab_section->sh_link];
    const char* other_strtab_data = (char*)other_file->map_start + other_strtab->sh_offset;
    Elf32_Sym* other_symbols = (Elf32_Sym*)((char*)other_file->map_start + 
                                           other_symtab_section->sh_offset);
    int other_sym_count = other_symtab_section->sh_size / sizeof(Elf32_Sym);

    // Process each symbol (skip symbol 0)
    for (int i = 1; i < curr_sym_count; i++) {
        Elf32_Sym* curr_sym = &curr_symbols[i];
        
        // Skip non-global and unnamed symbols
        if (!is_symbol_global(curr_sym) || curr_sym->st_name == 0) continue;
        
        const char* sym_name = curr_strtab_data + curr_sym->st_name;
        if (sym_name[0] == '\0') continue;

        // Find this symbol in other file
        Elf32_Sym* other_sym = find_symbol_in_table(sym_name, other_symbols, 
                                                   other_sym_count, other_strtab_data);

        if (curr_sym->st_shndx == SHN_UNDEF) {
            // Symbol is undefined in current file
            if (!other_sym || other_sym->st_shndx == SHN_UNDEF) {
                // Only print undefined symbols when checking first file
                if (is_first_file) {
                    printf("Symbol %s undefined\n", sym_name);
                }
            }
        } else {
            // Symbol is defined in current file
            if (other_sym && other_sym->st_shndx != SHN_UNDEF) {
                // Print multiply defined symbols only once
                if (is_first_file) {
                    printf("Symbol %s multiply defined\n", sym_name);
                }
            }
        }
    }
}

void check_merge_conflicts(int* has_undefined, int* has_multiply_defined) {
    *has_undefined = 0;
    *has_multiply_defined = 0;
    
    // Get symbol tables
    Elf32_Shdr* symtab1 = find_symbol_table_section(&elf_files[0]);
    Elf32_Shdr* symtab2 = find_symbol_table_section(&elf_files[1]);
    
    if (!symtab1 || !symtab2) return;
    
    // Get string tables
    Elf32_Shdr* strtab1 = &((Elf32_Shdr*)((char*)elf_files[0].map_start + 
                           elf_files[0].elf_header->e_shoff))[symtab1->sh_link];
    Elf32_Shdr* strtab2 = &((Elf32_Shdr*)((char*)elf_files[1].map_start + 
                           elf_files[1].elf_header->e_shoff))[symtab2->sh_link];
    
    const char* strtab1_data = (char*)elf_files[0].map_start + strtab1->sh_offset;
    const char* strtab2_data = (char*)elf_files[1].map_start + strtab2->sh_offset;
    
    Elf32_Sym* symbols1 = (Elf32_Sym*)((char*)elf_files[0].map_start + symtab1->sh_offset);
    Elf32_Sym* symbols2 = (Elf32_Sym*)((char*)elf_files[1].map_start + symtab2->sh_offset);
    
    int sym_count1 = symtab1->sh_size / sizeof(Elf32_Sym);
    int sym_count2 = symtab2->sh_size / sizeof(Elf32_Sym);
    
    // Check for conflicts
    for (int i = 1; i < sym_count1; i++) {
        if (symbols1[i].st_name == 0) continue;  // Skip unnamed symbols
        
        const char* sym_name = &strtab1_data[symbols1[i].st_name];
        Elf32_Sym* sym2 = find_symbol_in_table(sym_name, symbols2, sym_count2, strtab2_data);
        
        if (symbols1[i].st_shndx == SHN_UNDEF) {
            if (!sym2 || sym2->st_shndx == SHN_UNDEF) {
                *has_undefined = 1;
                printf("Symbol %s undefined\n", sym_name);
            }
        } else if (sym2 && sym2->st_shndx != SHN_UNDEF) {
            *has_multiply_defined = 1;
            printf("Symbol %s multiply defined\n", sym_name);
        }
    }
}


// Main function for checking files for merge
void check_files_for_merge() {
    if (!validate_merge_requirements()) {
        return;
    }
    int has_undefined = 0;
    int has_multiply_defined = 0;
    
    if (debug_mode) {
        fprintf(stderr, "\nDebug: Starting merge check\n");
    }
    
    check_merge_conflicts(&has_undefined, &has_multiply_defined);
    
    if (!has_undefined && !has_multiply_defined) {
        printf("Files are mergeable\n");
    }

    // Check symbols in both directions, but only print messages for the first file
    check_symbol_conflicts(&elf_files[0], &elf_files[1], 1);  // true for first file
    check_symbol_conflicts(&elf_files[1], &elf_files[0], 0);  // false for second file
}

//==================================== 3.2 ==============================

// ===== Section Management Helper Functions =====

// Finds a section by its name in an ELF file
Elf32_Shdr* get_section_by_name(ElfFile* elf_file, const char* section_name) {
    if (debug_mode) {
        fprintf(stderr, "\nDebug: Looking for section: %s\n", section_name);
        fprintf(stderr, "Debug: Number of sections: %d\n", elf_file->elf_header->e_shnum);
        fprintf(stderr, "Debug: Section header offset: 0x%x\n", elf_file->elf_header->e_shoff);
    }

    // Get section headers
    Elf32_Shdr* sections = (Elf32_Shdr*)((char*)elf_file->map_start + 
                                        elf_file->elf_header->e_shoff);
    
    // Get the section header string table
    Elf32_Shdr* shstrtab_hdr = &sections[elf_file->elf_header->e_shstrndx];
    const char* shstrtab = (char*)elf_file->map_start + shstrtab_hdr->sh_offset;
    
    if (debug_mode) {
        fprintf(stderr, "Debug: String table section index: %d\n", elf_file->elf_header->e_shstrndx);
        fprintf(stderr, "Debug: String table offset: 0x%x\n", shstrtab_hdr->sh_offset);
        fprintf(stderr, "Debug: String table size: %d bytes\n", shstrtab_hdr->sh_size);
    }
    
    // Search for section by name
    for (int i = 0; i < elf_file->elf_header->e_shnum; i++) {
        const char* curr_name = shstrtab + sections[i].sh_name;
        if (debug_mode) {
            fprintf(stderr, "Debug: Section %d name offset: %d, name: %s\n", 
                    i, sections[i].sh_name, curr_name);
        }
        if (strcmp(curr_name, section_name) == 0) {
            if (debug_mode) {
                fprintf(stderr, "Debug: Found section at index %d\n", i);
                fprintf(stderr, "Debug: Section offset: 0x%x\n", sections[i].sh_offset);
                fprintf(stderr, "Debug: Section size: %d bytes\n", sections[i].sh_size);
            }
            return &sections[i];
        }
    }
    
    if (debug_mode) {
        fprintf(stderr, "Debug: Section %s not found\n", section_name);
    }
    return NULL;
}

// Writes section data to output file and returns offset where written
off_t write_section_data(int out_fd, ElfFile* elf_file, Elf32_Shdr* section) {
    // Get the current offset in the output file
    off_t current_offset = lseek(out_fd, 0, SEEK_CUR);
    if (current_offset == -1) {
        perror("Failed to get current offset");
        return -1;
    }

    // Get the section data from the mapped memory
    void* section_data = (char*)elf_file->map_start + section->sh_offset;

    // Write the section data to the output file
    ssize_t written = write(out_fd, section_data, section->sh_size);
    if (written != section->sh_size) {
        perror("Failed to write section data");
        return -1;
    }

    return current_offset;
}



// Merges corresponding sections from both files
off_t merge_sections(int out_fd, ElfFile* file1, ElfFile* file2, const char* section_name, Elf32_Shdr* new_section) {
   Elf32_Shdr* section1 = get_section_by_name(file1, section_name);
   Elf32_Shdr* section2 = get_section_by_name(file2, section_name);

   if (!section1) {
       return -1;
   }

   void* section1_data = (char*)file1->map_start + section1->sh_offset;
   void* section2_data = section2 ? (char*)file2->map_start + section2->sh_offset : NULL;

   // Calculate aligned offset
   off_t new_offset = lseek(out_fd, 0, SEEK_CUR);
   new_offset = (new_offset + section1->sh_addralign - 1) & ~(section1->sh_addralign - 1);
   lseek(out_fd, new_offset, SEEK_SET);

   // Check for identical sections
   if (section2 && section1->sh_size == section2->sh_size && 
       memcmp(section1_data, section2_data, section1->sh_size) == 0) {
       
       if (write(out_fd, section1_data, section1->sh_size) != section1->sh_size) {
           return -1;
       }
       new_section->sh_size = section1->sh_size;
   } else {
       if (write(out_fd, section1_data, section1->sh_size) != section1->sh_size) {
           return -1;
       }
       new_section->sh_size = section1->sh_size;
       
       if (section2 && !memcmp(section1_data, section2_data, section1->sh_size)) {
           if (write(out_fd, section2_data, section2->sh_size) != section2->sh_size) {
               return -1;
           }
           new_section->sh_size += section2->sh_size;
       }
   }

   new_section->sh_offset = new_offset;
   new_section->sh_name = section1->sh_name;
   new_section->sh_type = section1->sh_type;
   new_section->sh_flags = section1->sh_flags;
   new_section->sh_addr = section1->sh_addr;
   new_section->sh_link = section1->sh_link;
   new_section->sh_info = section1->sh_info;
   new_section->sh_addralign = section1->sh_addralign;
   new_section->sh_entsize = section1->sh_entsize;

   return new_offset;
}


// Copies non-mergeable sections from first file
int copy_non_mergeable_sections(int out_fd, Elf32_Shdr* new_sections) {
    const char* non_mergeable_sections[] = {".shstrtab", ".symtab"};
    int num_sections = 2;
    
    if (debug_mode) 
        fprintf(stderr, "\nCopying non-mergeable sections at new_sections: %p\n", (void*)new_sections);
    
    for (int i = 0; i < num_sections; i++) {
        if (debug_mode) 
            fprintf(stderr, "\nCopying non-mergeable section: %s\n",  non_mergeable_sections[i]);
        
        // Get ORIGINAL section to find its index
        Elf32_Shdr* orig_section = get_section_by_name(&elf_files[0], non_mergeable_sections[i]);
        if (orig_section) {
            // Calculate the index
            Elf32_Shdr* first_section = (Elf32_Shdr*)((char*)elf_files[0].map_start + 
                                                     elf_files[0].elf_header->e_shoff);
            size_t section_index = (orig_section - first_section);
            
            if (debug_mode) {
                fprintf(stderr, "\nFound section at index: %zu\n", section_index);
                fprintf(stderr, "\nUsing new_sections + %zu\n", section_index);
            }
            
            // Use the index with our new_sections array
            Elf32_Shdr* new_section = &new_sections[section_index];
            
            // Write section data and update offset
            off_t new_offset = write_section_data(out_fd, &elf_files[0], orig_section);
            if (new_offset == -1) {
                printf("Failed to copy section: %s\n", non_mergeable_sections[i]);
                return -1;
            }

            if (debug_mode) {
                fprintf(stderr, "\nSuccessfully wrote section at offset: 0x%lx\n", new_offset);
                fprintf(stderr, "\nOriginal section size: %d\n", orig_section->sh_size);
            }

            // Update new section header
            new_section->sh_offset = new_offset;
            new_section->sh_size = orig_section->sh_size;

            if (debug_mode) {
                fprintf(stderr, "\nUpdated section header for section %zu:\n", section_index);
                fprintf(stderr, "\n- New offset: 0x%x\n", (unsigned int)new_section->sh_offset);
                fprintf(stderr, "\n- Size: %d\n", new_section->sh_size);
            }
        }
    }
    return 0;
}

// ===== Merge Support Functions =====

// Validates merge possibility
int can_merge_files() {
    return validate_merge_requirements();
}

// Creates output file
int create_output_file() {
    int out_fd = open("out.ro", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) {
        perror("Failed to create output file");
    }
    return out_fd;
}

// Writes initial ELF header
int write_initial_header(int out_fd, Elf32_Ehdr* header) {
    if (write(out_fd, header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Failed to write ELF header");
        return -1;
    }
    return 0;
}

Elf32_Shdr* create_initial_sections(ElfFile* source_file) {
   if (debug_mode) {
       fprintf(stderr, "\nDebug: Creating initial section headers\n");
       fprintf(stderr, "Debug: Number of sections: %d\n", source_file->elf_header->e_shnum);
       fprintf(stderr, "Debug: Section header offset: 0x%x\n", source_file->elf_header->e_shoff);
   }

   int num_sections = source_file->elf_header->e_shnum;
   size_t section_size = sizeof(Elf32_Shdr) * num_sections;
   
   Elf32_Shdr* new_sections = malloc(section_size);
   if (!new_sections) {
       if (debug_mode) {
           fprintf(stderr, "Debug: Failed to allocate %zu bytes for section headers\n", section_size);
       }
       perror("Failed to allocate memory for sections");
       return NULL;
   }
   
   Elf32_Shdr* source_sections = (Elf32_Shdr*)((char*)source_file->map_start + 
                                               source_file->elf_header->e_shoff);
   
   // Make deep copy of section headers
   memcpy(new_sections, source_sections, section_size);
   
   if (debug_mode) {
       fprintf(stderr, "\nDebug: Section header details:\n");
       for (int i = 0; i < num_sections; i++) {
           fprintf(stderr, "Debug: Section %d:\n", i);
           fprintf(stderr, "  - Offset: 0x%x\n", new_sections[i].sh_offset);
           fprintf(stderr, "  - Size: %d bytes\n", new_sections[i].sh_size);
           fprintf(stderr, "  - Type: 0x%x\n", new_sections[i].sh_type);
           fprintf(stderr, "  - Link: %d\n", new_sections[i].sh_link);
           fprintf(stderr, "  - Info: %d\n", new_sections[i].sh_info);
       }
   }
   
   return new_sections;
}

int process_mergeable_sections(int out_fd, Elf32_Shdr* new_sections) {
    const char* mergeable_sections[] = {".text", ".data", ".rodata"};
    int num_mergeable_sections = sizeof(mergeable_sections) / sizeof(mergeable_sections[0]);
    
    for (int i = 0; i < num_mergeable_sections; i++) {
        const char* section_name = mergeable_sections[i];
        
        Elf32_Shdr* orig_section = get_section_by_name(&elf_files[0], section_name);
        if (!orig_section) continue;

        size_t section_index = (orig_section - (Elf32_Shdr*)((char*)elf_files[0].map_start + 
                                                     elf_files[0].elf_header->e_shoff));
        Elf32_Shdr* new_section = &new_sections[section_index];
        
        if (merge_sections(out_fd, &elf_files[0], &elf_files[1], section_name, new_section) == -1) {
            return -1;
        }
    }
    return 0;
}

int update_symbol_values(int out_fd, Elf32_Shdr* symtab1, Elf32_Shdr* symtab2) {
    if (debug_mode) {
        fprintf(stderr, "\nDebug: Updating symbol values\n");
        fprintf(stderr, "Debug: First symtab size: %d bytes\n", symtab1->sh_size);
        fprintf(stderr, "Debug: Second symtab size: %d bytes\n", symtab2->sh_size);
    }

    // Get source symbol tables
    Elf32_Sym* source_symbols1 = (Elf32_Sym*)((char*)elf_files[0].map_start + symtab1->sh_offset);
    Elf32_Sym* source_symbols2 = (Elf32_Sym*)((char*)elf_files[1].map_start + symtab2->sh_offset);
    
    // Get string tables
    Elf32_Shdr* strtab1 = &((Elf32_Shdr*)((char*)elf_files[0].map_start + 
                           elf_files[0].elf_header->e_shoff))[symtab1->sh_link];
    Elf32_Shdr* strtab2 = &((Elf32_Shdr*)((char*)elf_files[1].map_start + 
                           elf_files[1].elf_header->e_shoff))[symtab2->sh_link];
    
    const char* strtab1_data = (char*)elf_files[0].map_start + strtab1->sh_offset;
    const char* strtab2_data = (char*)elf_files[1].map_start + strtab2->sh_offset;

    size_t sym_size = sizeof(Elf32_Sym);
    int sym_count = symtab1->sh_size / sym_size;
    
    // Create new symbol table with memory initialized to 0
    Elf32_Sym* new_symbols = calloc(sym_count, sym_size);
    if (!new_symbols) {
        if (debug_mode) fprintf(stderr, "Debug: Failed to allocate symbol table\n");
        return -1;
    }

    // Copy first file's symbols
    memcpy(new_symbols, source_symbols1, symtab1->sh_size);

    if (debug_mode) {
        fprintf(stderr, "Debug: Processing %d symbols\n", sym_count);
    }

    // Process undefined symbols
    for (int i = 0; i < sym_count; i++) {
        if (i == 0) continue; // Skip dummy symbol

        if (ELF32_ST_BIND(new_symbols[i].st_info) != STB_GLOBAL || 
            new_symbols[i].st_name == 0) {
            continue;
        }

        if (new_symbols[i].st_shndx == SHN_UNDEF) {
            const char* sym_name = &strtab1_data[new_symbols[i].st_name];
            
            if (debug_mode) {
                fprintf(stderr, "Debug: Looking for undefined symbol: %s\n", sym_name);
            }

            // Search in second file
            int sym_count2 = symtab2->sh_size / sym_size;
            for (int j = 1; j < sym_count2; j++) {
                const char* sym_name2 = &strtab2_data[source_symbols2[j].st_name];
                
                if (strcmp(sym_name, sym_name2) == 0 && 
                    source_symbols2[j].st_shndx != SHN_UNDEF) {
                    
                    if (debug_mode) {
                        fprintf(stderr, "Debug: Found symbol definition in second file\n");
                        fprintf(stderr, "Debug: Copying symbol attributes\n");
                    }

                    // Copy attributes from defined symbol
                    new_symbols[i].st_value = source_symbols2[j].st_value;
                    new_symbols[i].st_size = source_symbols2[j].st_size;
                    new_symbols[i].st_info = source_symbols2[j].st_info;
                    new_symbols[i].st_other = source_symbols2[j].st_other;
                    new_symbols[i].st_shndx = source_symbols2[j].st_shndx;
                    break;
                }
            }
        }
    }

    if (debug_mode) fprintf(stderr, "Debug: Writing updated symbol table\n");

    // Write updated table
    ssize_t written = write(out_fd, new_symbols, symtab1->sh_size);
    free(new_symbols);

    if (written != symtab1->sh_size) {
        if (debug_mode) fprintf(stderr, "Debug: Failed to write symbol table\n");
        return -1;
    }

    return 0;
}

off_t write_section_headers(int out_fd, Elf32_Shdr* new_sections, int num_sections) {
   if (debug_mode) {
       fprintf(stderr, "\nDebug: Writing section headers\n");
       fprintf(stderr, "Debug: Number of sections: %d\n", num_sections);
       fprintf(stderr, "Debug: Size of each header: %zu bytes\n", sizeof(Elf32_Shdr));
       fprintf(stderr, "Debug: Total size: %zu bytes\n", sizeof(Elf32_Shdr) * num_sections);
   }

   // Get aligned offset for section headers
   off_t current = lseek(out_fd, 0, SEEK_CUR);
   if (current == -1) {
       if (debug_mode) {
           fprintf(stderr, "Debug: Failed to get current file offset\n");
       }
       perror("Failed to get current offset");
       return -1;
   }

   if (debug_mode) {
       fprintf(stderr, "Debug: Writing section headers at offset 0x%lx\n", current);
       fprintf(stderr, "\nDebug: Section header details before write:\n");
       for (int i = 0; i < num_sections; i++) {
           fprintf(stderr, "Debug: Section %d:\n", i);
           fprintf(stderr, "  - Offset: 0x%x\n", new_sections[i].sh_offset);
           fprintf(stderr, "  - Size: %d bytes\n", new_sections[i].sh_size);
           fprintf(stderr, "  - Type: 0x%x\n", new_sections[i].sh_type);
           fprintf(stderr, "  - Link: %d\n", new_sections[i].sh_link);
           fprintf(stderr, "  - Info: %d\n", new_sections[i].sh_info);
       }
   }

   // Write section headers
   ssize_t written = write(out_fd, new_sections, sizeof(Elf32_Shdr) * num_sections);
   if (written != sizeof(Elf32_Shdr) * num_sections) {
       if (debug_mode) {
           fprintf(stderr, "Debug: Failed writing section headers\n");
           fprintf(stderr, "Debug: Expected %zu bytes, wrote %zd bytes\n", 
                   sizeof(Elf32_Shdr) * num_sections, written);
       }
       perror("Failed to write section headers");
       return -1;
   }

   if (debug_mode) {
       fprintf(stderr, "Debug: Successfully wrote %zd bytes of section headers\n", written);
   }

   return current;
}

int update_elf_header(int out_fd, Elf32_Ehdr* header, off_t section_offset) {
    header->e_shoff = section_offset;
    if (lseek(out_fd, 0, SEEK_SET) == -1) return -1;
    if (write(out_fd, header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) return -1;
    return 0;
}

void merge_elf_files() {
    if (!validate_merge_requirements()) {
        return;
    }

    if (debug_mode) {
        fprintf(stderr, "\nDebug: Starting ELF merge\n");
    }

    // Section order definition
    const char* section_order[] = {
        "",         // NULL section
        ".text",
        ".rel.text",
        ".rodata", 
        ".data",
        ".shstrtab",
        ".symtab",
        ".strtab"
    };
    const int num_ordered_sections = sizeof(section_order) / sizeof(section_order[0]);

    // Create output file
    int out_fd = create_output_file();
    if (out_fd == -1) return;

    if (debug_mode) {
        fprintf(stderr, "Debug: Created output file\n");
    }

    // Write initial ELF header
    Elf32_Ehdr new_header = *(elf_files[0].elf_header);
    if (write_initial_header(out_fd, &new_header) != 0) {
        if (debug_mode) fprintf(stderr, "Debug: Failed to write ELF header\n");
        close(out_fd);
        return;
    }

    // Create initial section headers
    Elf32_Shdr* new_sections = create_initial_sections(&elf_files[0]);
    if (!new_sections) {
        close(out_fd);
        return;
    }

    // Process sections in order
    for (int i = 0; i < num_ordered_sections; i++) {
        const char* section_name = section_order[i];
        
        if (debug_mode) {
            fprintf(stderr, "\nDebug: Processing section: %s\n", section_name);
        }
        
        Elf32_Shdr* section1 = get_section_by_name(&elf_files[0], section_name);
        if (!section1) continue;

        Elf32_Shdr* first_section = (Elf32_Shdr*)((char*)elf_files[0].map_start + 
                                                  elf_files[0].elf_header->e_shoff);
        size_t section_index = section1 - first_section;
        Elf32_Shdr* new_section = &new_sections[section_index];

        if (strcmp(section_name, ".text") == 0 ||
            strcmp(section_name, ".data") == 0 ||
            strcmp(section_name, ".rodata") == 0) {
            
            if (merge_sections(out_fd, &elf_files[0], &elf_files[1], 
                             section_name, new_section) == -1) {
                if (debug_mode) {
                    fprintf(stderr, "Debug: Failed to merge section %s\n", section_name);
                }
                free(new_sections);
                close(out_fd);
                return;
            }
        } else if (strcmp(section_name, ".symtab") == 0) {
            Elf32_Shdr* symtab2 = find_symbol_table_section(&elf_files[1]);
            if (symtab2) {
                if (update_symbol_values(out_fd, section1, symtab2) != 0) {
                    if (debug_mode) {
                        fprintf(stderr, "Debug: Failed to update symbol table\n");
                    }
                    free(new_sections);
                    close(out_fd);
                    return;
                }
                new_section->sh_offset = lseek(out_fd, 0, SEEK_CUR) - section1->sh_size;
            }
        } else {
            off_t new_offset = write_section_data(out_fd, &elf_files[0], section1);
            if (new_offset == -1) {
                if (debug_mode) {
                    fprintf(stderr, "Debug: Failed to write section %s\n", section_name);
                }
                free(new_sections);
                close(out_fd);
                return;
            }
            new_section->sh_offset = new_offset;
        }
    }

    // Write section headers and update ELF header
    off_t section_offset = write_section_headers(out_fd, new_sections, 
                                               elf_files[0].elf_header->e_shnum);
    if (section_offset == -1 || 
        update_elf_header(out_fd, &new_header, section_offset) != 0) {
        if (debug_mode) fprintf(stderr, "Debug: Failed to update headers\n");
        free(new_sections);
        close(out_fd);
        return;
    }

    free(new_sections);
    close(out_fd);

    if (debug_mode) {
        fprintf(stderr, "Debug: Merge completed successfully\n");
    }
    printf("Merged ELF files successfully\n");
}

//============================= close ===========================================

// Unmaps memory region mapped by mmap and closes the file descriptor to ensure proper resource deallocation.
void unmap_and_close(ElfFile *elf_file) {
    if (elf_file->map_start) {
        munmap(elf_file->map_start, elf_file->file_size);  // Unmaps the memory region.
        elf_file->map_start = NULL;
    }
    if (elf_file->fd >= 0) {
        close(elf_file->fd);  // Closes the file descriptor.
        elf_file->fd = -1;
    }
}

// Unmaps any resources and exits the program.
void quit() {
    for (int i = 0; i < file_count; i++) {
        unmap_and_close(&elf_files[i]);
    }
    printf("Exiting program.\n");
    exit(0);
}

struct fun_desc menu[] = { 
    {"Toggle Debug Mode", toggle_debug_mode},
        {"Examine ELF File", examine_elf_file},
        {"Print Section Names", print_section_names},
        {"Print Symbols", print_symbols},
        {"Check Files for Merge", check_files_for_merge},
        {"Merge ELF Files", merge_elf_files},
        {"Quit", quit},
        {NULL, NULL}

};

void display_menu(void) {
    printf("\nChoose action:\n");
    for(int i = 0; menu[i].name != NULL; i++) {
        printf("%d-%s\n", i, menu[i].name);
    }
}

// Main function presenting a menu for user actions.
int main() {
    //int bounds = (sizeof(menu) / sizeof(menu[0])) - 1; // -1 for NULL terminator
    state s = {
        .debug_mode = 0,
        .unit_size = 1,
        .display_mode = 0,
        .mem_count = 0,
        .file_name = ""
    };
       
    while (1) {
        if(s.debug_mode) {
            print_debug_info(&s);
        }

        display_menu();
        printf("Option: ");
        int option;
        scanf("%d", &option);
        getchar();  // Consume newline character.

        if (option >= 0 && menu[option].func != NULL) {
            menu[option].func();
        } else {
            printf("Invalid option\n");
        }
    }
    return 0;
}