#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>
#include <string.h>

#define MAX_FILES 2

// Define menu_item struct
typedef struct {
    char *name;
    void (*fun)();
} menu_item;

// Define state struct
typedef struct {
    int debug_mode;
    int fd[MAX_FILES];
    void *map_start[MAX_FILES];
    size_t file_size[MAX_FILES];
    Elf32_Ehdr *elf_header[MAX_FILES];
    int file_count;
} state;

state s = {0, {-1, -1}, {NULL, NULL}, {0, 0}, {NULL, NULL}, 0};

// Function prototypes
void toggle_debug_mode();
void unmap_and_close_files();
void quit();
void examine_elf_file();
void print_section_names();
void print_symbols();
void check_files_for_merge();
void merge_elf_files();
void print_menu();

// Menu array
menu_item menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Examine ELF File", examine_elf_file},
    {"Print Section Names", print_section_names},
    {"Print Symbols", print_symbols},
    {"Check Files for Merge", check_files_for_merge},
    {"Merge ELF Files", merge_elf_files},
    {"Quit", quit},
    {NULL, NULL}
};

void toggle_debug_mode() {
    s.debug_mode = !s.debug_mode;
    if (s.debug_mode) {
        fprintf(stderr, "Debug mode is now ON\n");
    } else {
        fprintf(stderr, "Debug mode is now OFF\n");
    }
}

void unmap_and_close_files() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (s.map_start[i] != NULL) {
            munmap(s.map_start[i], s.file_size[i]);
            s.map_start[i] = NULL;
        }
        if (s.fd[i] != -1) {
            close(s.fd[i]);
            s.fd[i] = -1;
        }
    }
}

void quit() {
    unmap_and_close_files();
    printf("Exiting...\n");
    exit(0);
}

void examine_elf_file() {
    if (s.file_count >= MAX_FILES) {
        printf("Error: Can only handle up to %d ELF files\n", MAX_FILES);
        return;
    }

    char file_name[256];
    printf("Enter ELF file name: ");
    scanf("%s", file_name);

    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror("open Error");
        return;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat Error");
        close(fd);
        return;
    }

    void *map_start = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    if (memcmp(header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Not an ELF file\n");
        munmap(map_start, sb.st_size);
        close(fd);
        return;
    }

    printf("Magic: %.4s\n", header->e_ident);
    printf("Data encoding: %d\n", header->e_ident[EI_DATA]);
    printf("Entry point: 0x%x\n", header->e_entry);
    printf("Section header offset: %d\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of section header entry: %d\n", header->e_shentsize);
    printf("Program header offset: %d\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of program header entry: %d\n", header->e_phentsize);

    s.fd[s.file_count] = fd;
    s.map_start[s.file_count] = map_start;
    s.file_size[s.file_count] = sb.st_size;
    s.elf_header[s.file_count] = header;
    s.file_count++;
}

const char *section_type_names[] = {
    "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", "DYNAMIC", "NOTE",
    "NOBITS", "REL", "SHLIB", "DYNSYM", "INIT_ARRAY", "FINI_ARRAY", "PREINIT_ARRAY",
    "GROUP", "SYMTAB_SHNDX", "NUM"
};

const char *get_section_type_name(uint32_t type) {
    if (type < sizeof(section_type_names) / sizeof(section_type_names[0])) {
        return section_type_names[type];
    }
    return "UNKNOWN";
}

void print_section_names() {
    if (s.file_count == 0) {
        printf("No ELF files currently loaded.\n");
        return;
    }

    for (int i = 0; i < s.file_count; i++) {
        Elf32_Ehdr *header = s.elf_header[i];
        Elf32_Shdr *sections = (Elf32_Shdr *)(s.map_start[i] + header->e_shoff);
        const char *strtab = s.map_start[i] + sections[header->e_shstrndx].sh_offset;

        printf("File %d\n", i + 1);
        for (int j = 0; j < header->e_shnum; j++) {
            Elf32_Shdr *sec = &sections[j];
            printf("[%2d] %-20s %08x %06x %06x %-12s\n",
                   j,
                   &strtab[sec->sh_name],
                   sec->sh_addr,
                   sec->sh_offset,
                   sec->sh_size,
                   get_section_type_name(sec->sh_type));
        }
    }
}

void print_symbols() {
    if (s.file_count == 0) {
        printf("No ELF files currently loaded.\n");
        return;
    }

    for (int i = 0; i < s.file_count; i++) {
        Elf32_Ehdr *header = s.elf_header[i];
        Elf32_Shdr *sections = (Elf32_Shdr *)(s.map_start[i] + header->e_shoff);

        int symtab_idx = -1;
        int strtab_idx = -1;

        for (int j = 0; j < header->e_shnum; j++) {
            if (sections[j].sh_type == SHT_SYMTAB) {
                symtab_idx = j;
            }
            if (sections[j].sh_type == SHT_STRTAB && strcmp((char *)s.map_start[i] + sections[header->e_shstrndx].sh_offset + sections[j].sh_name, ".strtab") == 0) {
                strtab_idx = j;
            }
        }

        if (symtab_idx == -1 || strtab_idx == -1) {
            printf("No symbol table found in file %d\n", i + 1);
            continue;
        }

        Elf32_Sym *symtab = (Elf32_Sym *)(s.map_start[i] + sections[symtab_idx].sh_offset);
        const char *strtab = s.map_start[i] + sections[strtab_idx].sh_offset;
        int num_symbols = sections[symtab_idx].sh_size / sizeof(Elf32_Sym);

        printf("File %d\n", i + 1);
        for (int j = 0; j < num_symbols; j++) {
            Elf32_Sym *sym = &symtab[j];
            const char *sym_name = strtab + sym->st_name;
            const char *sec_name = "UND";

            if (sym->st_shndx != SHN_UNDEF && sym->st_shndx < header->e_shnum) {
                sec_name = (char *)(s.map_start[i] + sections[header->e_shstrndx].sh_offset + sections[sym->st_shndx].sh_name);
            }

            printf("[%2d] %08x %2d %-20s %s\n",
                   j,
                   sym->st_value,
                   sym->st_shndx,
                   sec_name,
                   sym_name);
        }
    }
}

void check_files_for_merge() {
    if (s.file_count < 2) {
        printf("Error: Two ELF files must be opened and mapped.\n");
        return;
    }

    Elf32_Shdr *sections1 = (Elf32_Shdr *)(s.map_start[0] + s.elf_header[0]->e_shoff);
    Elf32_Shdr *sections2 = (Elf32_Shdr *)(s.map_start[1] + s.elf_header[1]->e_shoff);

    int symtab_idx1 = -1, strtab_idx1 = -1;
    int symtab_idx2 = -1, strtab_idx2 = -1;

    for (int i = 0; i < s.elf_header[0]->e_shnum; i++) {
        if (sections1[i].sh_type == SHT_SYMTAB) {
            symtab_idx1 = i;
        }
        if (sections1[i].sh_type == SHT_STRTAB && strcmp((char *)s.map_start[0] + sections1[s.elf_header[0]->e_shstrndx].sh_offset + sections1[i].sh_name, ".strtab") == 0) {
            strtab_idx1 = i;
        }
    }

    for (int i = 0; i < s.elf_header[1]->e_shnum; i++) {
        if (sections2[i].sh_type == SHT_SYMTAB) {
            symtab_idx2 = i;
        }
        if (sections2[i].sh_type == SHT_STRTAB && strcmp((char *)s.map_start[1] + sections2[s.elf_header[1]->e_shstrndx].sh_offset + sections2[i].sh_name, ".strtab") == 0) {
            strtab_idx2 = i;
        }
    }

    if (symtab_idx1 == -1 || strtab_idx1 == -1 || symtab_idx2 == -1 || strtab_idx2 == -1) {
        printf("Feature not supported: Each file must have exactly one symbol table and one string table.\n");
        return;
    }

    Elf32_Sym *symtab1 = (Elf32_Sym *)(s.map_start[0] + sections1[symtab_idx1].sh_offset);
    Elf32_Sym *symtab2 = (Elf32_Sym *)(s.map_start[1] + sections2[symtab_idx2].sh_offset);
    const char *strtab1 = s.map_start[0] + sections1[strtab_idx1].sh_offset;
    const char *strtab2 = s.map_start[1] + sections2[strtab_idx2].sh_offset;
    int num_symbols1 = sections1[symtab_idx1].sh_size / sizeof(Elf32_Sym);
    int num_symbols2 = sections2[symtab_idx2].sh_size / sizeof(Elf32_Sym);

    for (int i = 1; i < num_symbols1; i++) {
        Elf32_Sym *sym1 = &symtab1[i];
        const char *sym_name1 = strtab1 + sym1->st_name;
        if (sym1->st_name == 0) continue; // Skip unnamed symbols
        if (sym1->st_shndx == SHN_UNDEF) {
            int found = 0;
            for (int j = 1; j < num_symbols2; j++) {
                Elf32_Sym *sym2 = &symtab2[j];
                const char *sym_name2 = strtab2 + sym2->st_name;
                if (strcmp(sym_name1, sym_name2) == 0) {
                    if (sym2->st_shndx != SHN_UNDEF) {
                        found = 1;
                        break;
                    }
                }
            }
            if (!found) {
                printf("Symbol %s undefined.\n", sym_name1);
            }
        } else {
            for (int j = 1; j < num_symbols2; j++) {
                Elf32_Sym *sym2 = &symtab2[j];
                const char *sym_name2 = strtab2 + sym2->st_name;
                if (strcmp(sym_name1, sym_name2) == 0 && sym2->st_shndx != SHN_UNDEF) {
                    printf("Symbol %s multiply defined.\n", sym_name1);
                }
            }
        }
    }
}

void merge_elf_files() {
    if (s.file_count < 2) {
        printf("Error: Two ELF files must be opened and mapped.\n");
        return;
    }

    int out_fd = open("out.ro", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (out_fd == -1) {
        perror("Failed to create output file");
        return;
    }

    Elf32_Ehdr *elf1 = s.elf_header[0];
    Elf32_Ehdr *elf2 = s.elf_header[1];

    write(out_fd, elf1, sizeof(Elf32_Ehdr));

    // Merge sections (simplified for demonstration)
    Elf32_Shdr *sections1 = (Elf32_Shdr *)(s.map_start[0] + elf1->e_shoff);
    Elf32_Shdr *sections2 = (Elf32_Shdr *)(s.map_start[1] + elf2->e_shoff);

    // Example: Write .text section
    // Repeat similar logic for .data, .rodata, and other sections

    close(out_fd);
}

void print_menu() {
    for (int i = 0; menu[i].name != NULL; i++) {
        printf("%d-%s\n", i, menu[i].name);
    }
}

int main() {
    while (1) {
        if (s.debug_mode) {
            fprintf(stderr, "Debug: file_count=%d\n", s.file_count);
        }
        printf("Choose action:\n");
        print_menu();
        int choice;
        scanf("%d", &choice);
        getchar(); // Consume newline character left in the buffer
        if (choice >= 0 && choice < 7) {
            menu[choice].fun();
        } else {
            printf("Invalid choice\n");
        }
    }
    return 0;
}
