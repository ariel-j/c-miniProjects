#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN 5 

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};

int bigEndianFlag = 0;
link* virus_list_global = NULL; 

int isLittleEndian(unsigned char magic[]){
    return (magic[0] == 'V' && magic[1] == 'I' && magic[2] == 'R' && magic[3] == 'L');
}

int isBigEndian(unsigned char magic[]){
    if(magic[0] == 'V' && magic[1] == 'I' && magic[2] == 'R' && magic[3] == 'B'){
        bigEndianFlag = 1;
        return 1;
    }
    return 0;
}

int checkEndian(unsigned char magic[]){
   // printf("Magic number bytes: %02X %02X %02X %02X\n", magic[0], magic[1], magic[2], magic[3]);
    return (isLittleEndian(magic) || isBigEndian(magic));
}

int checkMagicNumber(FILE* virusFile) {
    unsigned char magic[4];
    size_t bytesRead = fread(magic, sizeof(unsigned char), 4, virusFile);
    if (bytesRead != 4 || !checkEndian(magic)) {
        fprintf(stderr, "Invalid magic number.\n");
        return 0;
    }
    return 1;
}

int readVirusDescription(FILE* virusFile, virus* v) {

    if (fread(&v->SigSize, sizeof(unsigned short), 1, virusFile) != 1 
    ||  fread(v->virusName, sizeof(char), 16, virusFile) != 16) {
        fprintf(stderr, "Error reading virus header.\n");
        return 0;
    }


    unsigned char* rawBytes = (unsigned char*)&v->SigSize;
    printf("Raw bytes: %02X %02X\n", rawBytes[0], rawBytes[1]);
    if(bigEndianFlag){
        // Print original value before conversion
        printf("Original SigSize: 0x%04X\n", v->SigSize);
        
        // Alternative byte swap method
        v->SigSize = ((v->SigSize & 0xFF00) >> 8) | ((v->SigSize & 0x00FF) << 8);
        
        // Print converted value
        printf("Converted SigSize: 0x%04X\n", v->SigSize);
    }


    
    // Debug: Print the SigSize value to check its validity
    printf("SigSize read: %u bytes\n", v->SigSize);
    v->virusName[15] = '\0'; 

    v->sig = (unsigned char*)malloc(v->SigSize);
    if (!v->sig) {
        fprintf(stderr, "Memory allocation failed for virus signature.\n");
        return 0;
    }

    return 1;
}

int readVirusSignature(FILE* virusFile, virus* v) {

    size_t bytesRead = fread(v->sig, sizeof(unsigned char), v->SigSize, virusFile);
    
    // Debugging: Print the bytes of the signature read (before error check)
    printf("Read %zu bytes of virus signature: ", bytesRead);
    for (unsigned int i = 0; i < bytesRead; i++) {
        printf("%02X ", v->sig[i]); // Print each byte in hexadecimal format
    }
    printf("\n");

    if (bytesRead != v->SigSize) {
        fprintf(stderr, "Error reading virus signature. Expected %u bytes, but read %zu bytes.\n", v->SigSize, bytesRead);
        free(v->sig);
        return 0;
    }

    return 1;
}

/**
 * this function receives a file pointer and returns a virus* that represents the next virus in the file. 
 * To read from a file, use fread(). See man fread(3) for assistance.
 */
virus* readVirus(FILE* virusFile){

    //if (!checkMagicNumber(virusFile)) return NULL; <---- already done!

    virus* v = (virus*)malloc(sizeof(virus));
    if (!v) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }

    if (!readVirusDescription(virusFile, v)) {
        free(v);
        return NULL;
    }

    if (!readVirusSignature(virusFile, v)) {
        free(v);
        return NULL;
    }

    return v;
}

/**
 * void printVirus(virus* virus, FILE* output): this function receives a virus and a pointer to an output file. The function prints the virus to the given output. 
 * It prints the virus name (in ASCII), the virus signature length (in decimal), and the virus signature (in hexadecimal representation).
 */
void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %u\n", virus->SigSize);
    fprintf(output, "signature:\n");
    for (int i = 0; i < virus->SigSize; i++) {
        fprintf(output, "%02X ", virus->sig[i]);
    }
    fprintf(output, "\n");
}

/**
 * Print the data of every link in list to the given stream. Each item followed by a newline character. 
 */
void list_print(link *virus_list, FILE* stream){
    while(virus_list != NULL){
        printVirus(virus_list->vir,stream);
        virus_list = virus_list->nextVirus;
    }
}

/**
 * Add a new link with the given data to the list (at the end CAN ALSO AT BEGINNING),
 * and return a pointer to the list (i.e., the first link in the list).
 * If the list is null - create a new entry and return a pointer to the entry.
*/
link* list_append(link* virus_list, virus* data){

    link* new_link = (link*)malloc(sizeof(link));
    if(!new_link){
        fprintf(stderr,"malloc failed!");
        return NULL;
    }

    new_link->vir = data;
    new_link->nextVirus = NULL;

    if(virus_list == NULL) return new_link;

    link* temp = virus_list; // Using temp so virus_list will point at the head.
    while(temp -> nextVirus != NULL){
        temp = temp->nextVirus;
    }
    temp->nextVirus = new_link;
    printf("appended!");
    return virus_list;
}

/**
 * Free the memory allocated by the list
 */
void list_free(link *virus_list){
    while(virus_list != NULL){
        link* temp = virus_list;
        free (temp->vir->sig);
        free (temp->vir);
        virus_list = virus_list->nextVirus;
        free(temp);
    }
}

void loadViruses(FILE* file){
    bigEndianFlag = 0;

    if (!checkMagicNumber(file)) {
        fclose(file);
        return;
    }

    virus* v;
    while ((v = readVirus(file)) != NULL) {
        virus_list_global = list_append(virus_list_global, v);
    }

    if (ferror(file)) {
        fprintf(stderr, "Error while reading signatures file.\n");
        list_free(virus_list_global);
        virus_list_global = NULL;
    }
}

int checkFileName(char input[],char filename[]){
    if (sscanf(input, "%s", filename) != 1) {
        fprintf(stderr, "Invalid filename\n");
        return 0;
    }
    return 1;
}

void print_signatures() {
    if (virus_list_global == NULL) {
        printf("No signatures loaded.\n");
        return;
    }
    list_print(virus_list_global, stdout);
}

/**
 * Load signatures function
 */ 
void load_signatures() {
    char input[256];
    char filename[256];
    
    printf("Enter signature file name: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        return;
    }
    
    if(!checkFileName(input,filename)) return;

    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file"); //If no file is loaded, nothing is printed.  <---- debug line
        return;
    }
    loadViruses(file);
    print_signatures();
    fclose(file);
    printf("Signatures loaded successfully\n");
}

void neutralize_virus(char *fileName, int signatureOffset);

void detect_virus(char* buffer, unsigned int size, link* virus_list, FILE* file,char* fileName) {
    link *current = virus_list;

    while (current != NULL) {
        virus *v = current->vir;
        if (v->sig == NULL || v->SigSize == 0) {
            fprintf(stderr, "Invalid virus signature.\n");
            current = current->nextVirus;
            continue;
        }

        for (unsigned int i = 0; i <= size - v->SigSize; i++) {
            if (memcmp(&buffer[i], v->sig, v->SigSize) == 0) {
                printf("Virus detected!\n");
                printf("Start byte location(hex): 0x%x\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus signature size: %u\n", v->SigSize);
            }
        }
        current = current->nextVirus;
    }
}

void detect_viruses() {
    if (virus_list_global == NULL) {
        printf("No virus signatures loaded.\n");
        return;
    }

    char filename[256];
    printf("Enter the suspected file name: ");
    if (scanf("%255s", filename) != 1) {
        printf("Invalid filename input.\n");
        return;
    }
    
    FILE* file = fopen(filename, "r+b");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char buffer[10240]; // 10K bytes
    size_t total_bytes_read = 0;
    while (1) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
        if (bytes_read == 0) break;

        detect_virus(buffer, bytes_read, virus_list_global,file,filename);

        total_bytes_read += bytes_read;
        if (bytes_read < sizeof(buffer)) break;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // clear input
    fclose(file);
}




void fix_file() {
    if (virus_list_global == NULL) {
        printf("No virus signatures loaded.\n");
        return;
    }

    char filename[256];
    printf("Enter the file to fix: ");
    if (scanf("%255s", filename) != 1) {
        printf("Invalid filename input.\n");
        return;
    }

    FILE* file = fopen(filename, "r+b");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char buffer[10240]; // 10K bytes
    size_t total_bytes_read = 0;
    int viruses_neutralized = 0;

    while (1) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
        if (bytes_read == 0) break;

        link *current = virus_list_global;
        while (current != NULL) {
            virus *v = current->vir;
            
            for (unsigned int i = 0; i <= bytes_read - v->SigSize; i++) {
                if (memcmp(&buffer[i], v->sig, v->SigSize) == 0) {
                    neutralize_virus(filename, total_bytes_read + i);
                    viruses_neutralized++;
                }
            }
            current = current->nextVirus;
        }

        total_bytes_read += bytes_read;
        if (bytes_read < sizeof(buffer)) break;
    }

    fclose(file);
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // clear input
    printf("Total viruses neutralized: %d\n", viruses_neutralized);
}

void quit(void) {
    printf("Exiting...\n");
    list_free(virus_list_global);
    virus_list_global = NULL;
    exit(0);
}

// =============================== TOOK FROM LAB 1 ==========================================/
struct fun_desc {
    char *name;
    void (*fun)();
};


struct fun_desc menu[] = {
    { "Load signatures", load_signatures },
    { "Print signatures", print_signatures },
    { "Detect viruses", detect_viruses },
    { "Fix file", fix_file },
    { "Quit", quit },
    { NULL, NULL }
};

void display_menu(struct fun_desc* menu){
    printf("Select operation from the following menu: \n");
    for(int i = 0; menu[i].name != NULL; i = i + 1){
        printf("%d) %s\n", i, menu[i].name);
    }
}

int getLength(char input []){
  int i = 0;
  while(input[i] != '\0'){i++;}
  return i;
}

int trim_newline_len(char input []){
    int len = getLength(input);
    if (len > 0 && input[len-1] == '\n') {
        input[len-1] = '\0';
        len--;
    }
    return len;
}

int validation(int len, char input []){
  if (len == 0 || len >= LEN - 1) return 0;
  for (int i = 0; input[i] != '\0'; i++) {
      if (input[i] < '0' || input[i] > '9') {
          return 0;
      }
  }
  return 1;
}

char * process_inputs(int bounds, char * carray, int size){
    char input[LEN];
    char * fgetsInput = fgets(input, LEN, stdin);
    if (!fgetsInput) {  
        exit(1);
    }

    int input_len = trim_newline_len(input); 

    int valid = validation(input_len,input); 

    int choice = atoi(input);
    if (!valid || choice < 0 || choice >= bounds) {
        printf("Invalid input. Please enter a valid menu option.\n");
        return carray;
    }

    printf("Option: %d\n", choice);
    menu[choice].fun();
    printf("DONE.\n");
    return carray;
}
// =============================== FINISHED TAKING FROM LAB 1 ==========================================/

void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *file = fopen(fileName, "r+b");
    if (!file) {
        perror("Error opening file");
        return;
    }

    if (fseek(file, signatureOffset, SEEK_SET) != 0) {
        perror("Error seeking to virus location");
        fclose(file);
        return;
    }

    unsigned char ret_instruction = 0xC3;//return near to caller
    if (fwrite(&ret_instruction, sizeof(unsigned char), 1, file) != 1) {
        perror("Error writing RET instruction");
    }

    fclose(file);
    printf("Virus neutralized at offset %d\n", signatureOffset);
}

int main(int argc, char **argv) {
    int bounds = (sizeof(menu) / sizeof(menu[0])) - 1;

    while (1) {
        display_menu(menu);
        process_inputs(bounds, NULL, 0);
    }
    return 0;
}

