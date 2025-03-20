#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isLittleEndian = 0; 
char *fileName = NULL;

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
} link;

struct fun_desc {
  char *name;
  void (*fun)(link** virus_list);
};

unsigned short swapBytes(unsigned short val) {
    return (val >> 8) | (val << 8);
}

// Viruses Functions
virus* readVirus(FILE* file) {
    virus* v = (virus*) malloc(sizeof(virus));
    if (fread(v, 1, 18, file) != 18) {
        free(v);
        return NULL;
    }
    if (!isLittleEndian) {
        v->SigSize = swapBytes(v->SigSize);
    }
    v->sig = (unsigned char *) malloc(v->SigSize); 
    if (!v->sig) {
        perror("Failed to allocate memory for signature");
        free(v);
        return NULL;
    }
    if (fread(v->sig, 1, v->SigSize, file) != v->SigSize) {
        free(v->sig);
        free(v);
        return NULL;
    }
    return v;
}

void printVirus(virus* v, FILE* output) {
    fprintf(output, "Virus name: %s\n", v->virusName);
    fprintf(output, "Virus signature size: %u\n", v->SigSize);
    fprintf(output, "Signature:\n");

    for (int i = 0; i < v->SigSize; i++) {
        fprintf(output, "%02X ", v->sig[i]);
    }

    fprintf(output, "\n\n");
}
void freeVirus(virus* v) {
        free(v->sig);
        free(v);
    }


// Linked list Functions
void list_print(link *virus_list, FILE *output) {
    link *current = virus_list;
    while (current != NULL) {
        printVirus(current->vir, output);
        current = current->nextVirus;
    }
}

link *list_append(link *virus_list, virus *data) {
    link *newLink = (link *)malloc(sizeof(link));
    if (!newLink) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    newLink->vir = data;
    newLink->nextVirus = virus_list;
    return newLink;
}

void list_free(link *virus_list) {
    link *current = virus_list;
    while (current != NULL) {
        link *next = current->nextVirus;
        freeVirus(current->vir);
        free(current);
        current = next;
    }
}

//menu Function

void detect_virus(unsigned char* buffer, unsigned int size, link* virus_list){
    while(virus_list){
        for(unsigned int i = 0; i < size - virus_list->vir->SigSize; i++){
            if(memcmp(virus_list->vir->sig, buffer + i, virus_list->vir->SigSize) == 0){
                printf("VirusName %s\nVirusSize %d\nVirusLocation %d\n", virus_list->vir->virusName, virus_list->vir->SigSize, i);
            }
        }
        virus_list = virus_list->nextVirus;
    }
}

void neutralize_virus(char* name, int signatureOffset){
    printf("killing virus at %d in file %s\n", signatureOffset, name);
    char c3 = 0xC3;
    FILE* file = fopen(name, "rb+");
    if(!file){
        perror("fopen");
        return;
    }
    fseek(file, signatureOffset, SEEK_SET);
    fwrite(&c3, 1, 1, file);
    fclose(file);
}

void detect_virus_then_fix(unsigned char* buffer, unsigned int size, link* virus_list){
    while(virus_list){
        for(unsigned int i = 0; i < size - virus_list->vir->SigSize; i++){
            if(memcmp(virus_list->vir->sig, buffer + i, virus_list->vir->SigSize) == 0){
                neutralize_virus(fileName, i);
            }
        }
        virus_list = virus_list->nextVirus;
    }
}


void Load_Signatures (link **virus_list){
    char filename[256];
    printf("Enter the signature file name: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("Failed to read file name.\n");
        return;
    }
    filename[strcspn(filename, "\n")] = '\0'; 
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    
    char magicNumber[5] = {0};
    if (fread(magicNumber, 1, 4, file) != 4) {
        fprintf(stderr, "Error reading magic number\n");
        fclose(file);
        return;
    }
    
    isLittleEndian = (strcmp(magicNumber, "VIRL") == 0);
    if (!isLittleEndian && strcmp(magicNumber, "VIRB") != 0) {
        printf("Invalid magic number: %s\n", magicNumber);
        fclose(file);
        return;
    }
    printf("Magic number verified: %s\n", magicNumber);
    
    list_free(*virus_list);
    *virus_list = NULL;

    virus *v;
    while ((v = readVirus(file)) != NULL) {
        *virus_list = list_append(*virus_list, v);
    }
    fclose(file);
    printf("Signatures loaded successfully.\n");
}

void Print_Signatures(link **virus_list) {
    if (*virus_list == NULL) {
        printf("No signatures loaded.\n");
        return;
    }
    list_print(*virus_list, stdout);
}

void Detect_viruses(link** virus_list) {
    FILE* file = fopen(fileName, "rb");
    unsigned char buffer[10000];
    if(!file){
        perror("fopen");
        return;
    }
    unsigned int size = fread(buffer, 1, 10000, file);
    fclose(file);
    detect_virus(buffer, size, *virus_list);
}

void Fix_File(link** virus_list) {
    FILE* file = fopen(fileName, "rb");
    unsigned char buffer[10000];
    if(!file){
        perror("fopen");
        return;
    }
    unsigned int size = fread(buffer, 1, 10000, file);
    fclose(file);
    detect_virus_then_fix(buffer, size, *virus_list);
}

void Quit_Function(link** virus_list) {
    list_free(*virus_list);
    exit(0);
}



int main(int argc, char** argv) {
    if(argc != 2){
        fprintf(stdout, "Usuage %s file_name\n", argv[0]);
        return 0;
    }
    char input[100];
    link *virus_list = NULL;
    fileName = argv[1];

    struct fun_desc menu[] = { 
     {"Load signatures", Load_Signatures}, 
     {"Print signatures", Print_Signatures}, 
     {"Detect viruses",Detect_viruses}, 
     {"Fix file", Fix_File}, 
     {"Quit",Quit_Function}, {NULL, NULL}};
    int len = sizeof(menu) / sizeof(menu[0]) - 1;
    while(1){
        printf("Select operation from the following menu:\n");

        for (int i = 0; i < len; i++){
            printf("%d", i);
            printf(") %s \n", menu[i].name);
        }
        printf("Option: ");
        if(fgets(input, 100, stdin) == NULL) break; 
        int num = atoi(input);
        printf("\n");
        if (num < 0 && num >= len){
            printf("not within bounds\n");
            exit(EXIT_FAILURE);
        }
        printf("within bounds\n");
        menu[num].fun(&virus_list);
    }
    list_free(virus_list);
    return 0;
}