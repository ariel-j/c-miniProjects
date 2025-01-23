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


// ============================== Error codes ==============================
typedef enum {
    SUCCESS = 0,
    ERROR_FILE_EMPTY = -1,
    ERROR_FILE_OPEN = -2,
    ERROR_INPUT = -3,
    ERROR_BOUNDS = -4,
    ERROR_LOCATION = -5
} error_code;

//============================== Format strings ==============================

// static const char* hex_formats[] = {"%hhx\n", "%hx\n", "No such unit", "%x\n"};
// static const char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

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

int trim_newline_len(char input[]) {
    int len = strlen(input);
    if (len > 0 && input[len-1] == '\n') {
        input[len-1] = '\0';
        len--;
    }
    return len;
}

error_code validate_input(const char* input, int* result, int min, int max) {
    if (!input || !result) return ERROR_INPUT;
    
    char* endptr;
    long val = strtol(input, &endptr, 10);
    
    if (endptr == input || *endptr != '\n') return ERROR_INPUT;
    if (val < min || val > max) return ERROR_BOUNDS;
    
    *result = (int)val;
    return SUCCESS;
}

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
void check_symbol_conflicts(ElfFile* curr_file, ElfFile* other_file) {
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
                printf("Symbol %s undefined\n", sym_name);
            }
        } else {
            // Symbol is defined in current file
            if (other_sym && other_sym->st_shndx != SHN_UNDEF) {
                printf("Symbol %s multiply defined\n", sym_name);
            }
        }
    }
}

// Main function for checking files for merge
void check_files_for_merge() {
    if (!validate_merge_requirements()) {
        return;
    }

    // Check symbols in both directions
    check_symbol_conflicts(&elf_files[0], &elf_files[1]);
    check_symbol_conflicts(&elf_files[1], &elf_files[0]);
}

//==================================== 3.2 ==============================

// ===== Section Management Helper Functions =====

// Finds a section by its name in an ELF file
Elf32_Shdr* get_section_by_name(ElfFile* elf_file, const char* section_name) {
    Elf32_Shdr* sections = (Elf32_Shdr*)((char*)elf_file->map_start + 
                                        elf_file->elf_header->e_shoff);
    
    // Get section names string table
    const char* shstrtab = (char*)elf_file->map_start + 
                          sections[elf_file->elf_header->e_shstrndx].sh_offset;
    
    // Search for section by name
    for (int i = 0; i < elf_file->elf_header->e_shnum; i++) {
        if (strcmp(shstrtab + sections[i].sh_name, section_name) == 0) {
            return &sections[i];
        }
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
off_t merge_sections(int out_fd, ElfFile* file1, ElfFile* file2, 
                     const char* section_name, Elf32_Shdr* new_section) {
    // Find the section in both files
    Elf32_Shdr* section1 = get_section_by_name(file1, section_name);
    Elf32_Shdr* section2 = get_section_by_name(file2, section_name);
    
    if (!section1) {
        // If the section doesn't exist in the first file, return error
        printf("Section %s not found in first file\n", section_name);
        return -1;
    }
    
    // Start writing the new merged section at the current offset
    off_t new_offset = lseek(out_fd, 0, SEEK_CUR);
    if (new_offset == -1) {
        perror("Failed to get current offset");
        return -1;
    }

    // Write section from the first file
    if (write_section_data(out_fd, file1, section1) == -1) {
        perror("Failed to write section from first file");
        return -1;
    }

    // If the section exists in the second file, append it
    if (section2) {
        if (write_section_data(out_fd, file2, section2) == -1) {
            perror("Failed to write section from second file");
            return -1;
        }

        // Update the size of the merged section (sum of both sections' sizes)
        new_section->sh_size = section1->sh_size + section2->sh_size;
    } else {
        // Only the first file has this section
        new_section->sh_size = section1->sh_size;
    }

    // Set the offset for the merged section
    new_section->sh_offset = new_offset;

    // Debug information
    if (debug_mode) {
        printf("Merged section %s:\n", section_name);
        printf("- New offset: 0x%lx\n", new_offset);
        printf("- New size: %d bytes\n", new_section->sh_size);
    }

    return new_offset;
}

// Copies non-mergeable sections from first file
int copy_non_mergeable_sections(int out_fd, Elf32_Shdr* new_sections) {
    const char* non_mergeable_sections[] = {".shstrtab", ".symtab"};
    int num_sections = 2;
    
    if (debug_mode) {
        printf("\nCopying non-mergeable sections at new_sections: %p\n", (void*)new_sections);
    }
    
    for (int i = 0; i < num_sections; i++) {
        if (debug_mode) {
            printf("Copying non-mergeable section: %s\n", non_mergeable_sections[i]);
        }
        
        // Get ORIGINAL section to find its index
        Elf32_Shdr* orig_section = get_section_by_name(&elf_files[0], non_mergeable_sections[i]);
        if (orig_section) {
            // Calculate the index
            Elf32_Shdr* first_section = (Elf32_Shdr*)((char*)elf_files[0].map_start + 
                                                     elf_files[0].elf_header->e_shoff);
            size_t section_index = (orig_section - first_section);
            
            if (debug_mode) {
                printf("Found section at index: %zu\n", section_index);
                printf("Using new_sections + %zu\n", section_index);
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
                printf("Successfully wrote section at offset: 0x%lx\n", new_offset);
                printf("Original section size: %d\n", orig_section->sh_size);
            }

            // Update new section header
            new_section->sh_offset = new_offset;
            new_section->sh_size = orig_section->sh_size;

            if (debug_mode) {
                printf("Updated section header:\n");
                printf("- New offset: 0x%x\n", (unsigned int)new_section->sh_offset);
                printf("- Size: %d\n", new_section->sh_size);
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

// Creates initial section headers
Elf32_Shdr* create_initial_sections(ElfFile* source_file) {
    int num_sections = source_file->elf_header->e_shnum;
    Elf32_Shdr* new_sections = malloc(sizeof(Elf32_Shdr) * num_sections);
    if (!new_sections) {
        perror("Failed to allocate memory for sections");
        return NULL;
    }
    
    memcpy(new_sections, 
           (char*)source_file->map_start + source_file->elf_header->e_shoff, 
           sizeof(Elf32_Shdr) * num_sections);
    
    return new_sections;
}

// Processes mergeable sections (.text, .data, .rodata)
int process_mergeable_sections(int out_fd, Elf32_Shdr* new_sections) {
    const char* mergeable_sections[] = {".text", ".data", ".rodata"};
    int num_mergeable_sections = sizeof(mergeable_sections) / sizeof(mergeable_sections[0]);

    for (int i = 0; i < num_mergeable_sections; i++) {
        const char* section_name = mergeable_sections[i];

        // Get the original section from the first file to calculate the index
        Elf32_Shdr* orig_section = get_section_by_name(&elf_files[0], section_name);
        if (!orig_section) {
            printf("Section %s not found in first file, skipping merge\n", section_name);
            continue;
        }

        // Calculate the section index in the new section header table
        Elf32_Shdr* first_section = (Elf32_Shdr*)((char*)elf_files[0].map_start + 
                                                  elf_files[0].elf_header->e_shoff);
        size_t section_index = orig_section - first_section;
        Elf32_Shdr* new_section = &new_sections[section_index];

        // Merge the sections
        if (merge_sections(out_fd, &elf_files[0], &elf_files[1], section_name, new_section) == -1) {
            printf("Failed to merge section: %s\n", section_name);
            return -1;
        }
    }

    return 0;
}


// Updates symbol values from second file
int update_symbol_values(int out_fd, Elf32_Shdr* symtab1, Elf32_Shdr* symtab2) {
    // Get source symbol tables
    Elf32_Sym* source_symbols1 = (Elf32_Sym*)((char*)elf_files[0].map_start + symtab1->sh_offset);
    Elf32_Sym* source_symbols2 = (Elf32_Sym*)((char*)elf_files[1].map_start + symtab2->sh_offset);
    
    int sym_count1 = symtab1->sh_size / sizeof(Elf32_Sym);
    int sym_count2 = symtab2->sh_size / sizeof(Elf32_Sym);

    // Create a copy of the first symbol table for the merged file
    Elf32_Sym* new_symbols = malloc(symtab1->sh_size);
    if (!new_symbols) {
        perror("Failed to allocate memory for symbol table");
        return -1;
    }
    memcpy(new_symbols, source_symbols1, symtab1->sh_size);

    // Get string tables
    Elf32_Shdr* strtab1 = &((Elf32_Shdr*)((char*)elf_files[0].map_start + 
                           elf_files[0].elf_header->e_shoff))[symtab1->sh_link];
    Elf32_Shdr* strtab2 = &((Elf32_Shdr*)((char*)elf_files[1].map_start + 
                           elf_files[1].elf_header->e_shoff))[symtab2->sh_link];

    const char* strtab1_data = (char*)elf_files[0].map_start + strtab1->sh_offset;
    const char* strtab2_data = (char*)elf_files[1].map_start + strtab2->sh_offset;

    // Update undefined symbols and handle multiply defined symbols
    for (int i = 1; i < sym_count1; i++) {
        Elf32_Sym* sym1 = &new_symbols[i];
        if (sym1->st_shndx != SHN_UNDEF) {
            // Symbol is already defined in the first file
            const char* sym_name = strtab1_data + sym1->st_name;

            // Search for this symbol in the second file
            Elf32_Sym* sym2 = find_symbol_in_table(sym_name, source_symbols2, sym_count2, strtab2_data);

            if (sym2 && sym2->st_shndx != SHN_UNDEF) {
                // Both files define the symbol; retain only one definition
                if (debug_mode) {
                    printf("Symbol %s multiply defined; retaining first definition\n", sym_name);
                }
                continue;  // Skip updating this symbol
            }
        } else {
            // Symbol is undefined in the first file; try to resolve it
            const char* sym_name = strtab1_data + sym1->st_name;
            Elf32_Sym* sym2 = find_symbol_in_table(sym_name, source_symbols2, sym_count2, strtab2_data);

            if (sym2 && sym2->st_shndx != SHN_UNDEF) {
                // Resolve the undefined symbol using the second file
                sym1->st_value = sym2->st_value;
                sym1->st_size = sym2->st_size;
                sym1->st_info = sym2->st_info;
                sym1->st_shndx = sym2->st_shndx;

                if (debug_mode) {
                    printf("Resolved undefined symbol %s using second file\n", sym_name);
                }
            }
        }
    }

    // Write the updated symbol table to the output file
    ssize_t written = write(out_fd, new_symbols, symtab1->sh_size);
    free(new_symbols);

    if (written != symtab1->sh_size) {
        perror("Failed to write updated symbol table");
        return -1;
    }
    return 0;
}

// Writes section headers and returns offset
off_t write_section_headers(int out_fd, Elf32_Shdr* new_sections, int num_sections) {
    if (debug_mode) {
        printf("\nWriting section headers:\n");
        printf("- Number of sections: %d\n", num_sections);
        printf("- Size of each section header: %zu\n", sizeof(Elf32_Shdr));
        printf("- Total size to write: %zu\n", sizeof(Elf32_Shdr) * num_sections);
    }

    // Make sure we're aligned
    off_t current = lseek(out_fd, 0, SEEK_CUR);
    if (current == -1) {
        perror("Failed to get current offset");
        return -1;
    }

    if (debug_mode) {
        printf("Current file offset before write: 0x%lx\n", current);
    }

    // Write the section headers
    ssize_t written = write(out_fd, new_sections, sizeof(Elf32_Shdr) * num_sections);
    if (written != sizeof(Elf32_Shdr) * num_sections) {
        perror("Failed to write section headers");
        return -1;
    }

    if (debug_mode) {
        printf("Successfully wrote %zd bytes of section headers\n", written);
    }

    return current;
}

// Updates ELF header
int update_elf_header(int out_fd, Elf32_Ehdr* header, off_t section_offset) {
    header->e_shoff = section_offset;
    if (lseek(out_fd, 0, SEEK_SET) == -1) return -1;
    if (write(out_fd, header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) return -1;
    return 0;
}

// Main merge function
void merge_elf_files() {
    // Step 1: Validation (allowing merge even if validation fails)
    if (!can_merge_files()) {
        printf("Warning: Merge validation failed! Continuing anyway as per requirements...\n");
    }
    
    // Step 2: Create output file
    int out_fd = create_output_file();
    if (out_fd == -1) return;

    // Step 3: Copy and setup initial ELF header
    Elf32_Ehdr new_header = *(elf_files[0].elf_header);
    if (write_initial_header(out_fd, &new_header) != 0) {
        close(out_fd);
        return;
    }

    // Step 4: Create initial section headers
    Elf32_Shdr* new_sections = create_initial_sections(&elf_files[0]);
    if (!new_sections) {
        close(out_fd);
        return;
    }

    // Step 5: Process mergeable sections
    if (process_mergeable_sections(out_fd, new_sections) != 0) {
        free(new_sections);
        close(out_fd);
        return;
    }

    // Step 6: Copy non-mergeable sections
    if (copy_non_mergeable_sections(out_fd, new_sections) != 0) {
        free(new_sections);
        close(out_fd);
        return;
    }

    // Step 7: Update symbol values
    Elf32_Shdr* symtab1 = find_symbol_table_section(&elf_files[0]);
    Elf32_Shdr* symtab2 = find_symbol_table_section(&elf_files[1]);
    if (symtab1 && symtab2) {
    if (update_symbol_values(out_fd, symtab1, symtab2) != 0) {
        printf("Failed to update symbol values\n");
        free(new_sections);
        close(out_fd);
        return;
    }
}

    // Step 8: Write final section headers and update ELF header
    off_t section_offset = write_section_headers(out_fd, new_sections, 
                                               elf_files[0].elf_header->e_shnum);
    if (section_offset == -1 || 
        update_elf_header(out_fd, &new_header, section_offset) != 0) {
        printf("Failed to update headers\n");
        free(new_sections);
        close(out_fd);
        return;
    }

    // Step 9: Cleanup and finish
    free(new_sections);
    close(out_fd);
    
    if (debug_mode) {
        printf("\nMerge completed:\n");
        printf("- Output file: out.ro\n");
        printf("- Section header table offset: 0x%x\n", (unsigned int)section_offset);
        printf("- Number of sections: %d\n", elf_files[0].elf_header->e_shnum);
    } else {
        printf("Merged ELF files successfully\n");
    }
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