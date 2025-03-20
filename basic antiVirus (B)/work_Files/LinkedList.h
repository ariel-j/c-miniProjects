#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

// Virus structure definition
typedef struct virus {
    unsigned short SigSize;      // Size of the virus signature
    char virusName[16];          // Name of the virus
    unsigned char *sig;          // Pointer to the signature data
} virus;

// Link structure for a linked list
typedef struct link {
    struct link *nextVirus;      // Pointer to the next link in the list
    virus *vir;                  // Pointer to a virus structure
} link;

// Function declarations
void print_signatures(link *virus_list);
void detect_viruses();
void fix_file();
void list_print(link *virus_list, FILE *output);
link* list_append(link *virus_list, virus *data);
void list_free(link *virus_list);
void load_signatures(link **virus_list);
virus *readVirus(FILE *file);
void printVirus(virus *v, FILE *output);

// Menu structure for function description
struct fun_desc {
    char *name;                  // Function name
    void (*fun)(link **);        // Pointer to the corresponding function
};

// Menu-related functions
void print_menu(struct fun_desc *menu);
void handle_choice(int choice, link **virus_list);

#endif // LINKEDLIST_H
