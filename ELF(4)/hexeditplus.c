#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

// ============================== Constants ==============================

#define MAX_INPUT_SIZE 100
#define MAX_BUFFER_SIZE 10000
#define MAX_FILENAME_SIZE 128
#define MIN_UNIT_SIZE 1
#define MAX_UNIT_SIZE 4

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

static const char* hex_formats[] = {"%hhx\n", "%hx\n", "No such unit", "%x\n"};
static const char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

// ============================== Structs ==============================

typedef struct {
    char debug_mode;
    char file_name[MAX_FILENAME_SIZE];
    int unit_size;
    unsigned char mem_buf[MAX_BUFFER_SIZE];
    size_t mem_count;
    char display_mode;
} state;

struct fun_desc {
    char *name;
    void (*fun)(state*);
};

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

// ============================== File Operations ==============================

FILE* open_file_read(const state* s, error_code* err) {
    if(strlen(s->file_name) == 0) {
        if(err) *err = ERROR_FILE_EMPTY;
        return NULL;
    }

    FILE* file = fopen(s->file_name, "rb");
    if(!file && err) *err = ERROR_FILE_OPEN;
    return file;
}

FILE* open_file_write(const state* s, error_code* err) {
    if(strlen(s->file_name) == 0) {
        if(err) *err = ERROR_FILE_EMPTY;
        return NULL;
    }

    FILE* file = fopen(s->file_name, "r+");
    if(!file && err) *err = ERROR_FILE_OPEN;
    return file;
}

// ============================== Display Functions ==============================

void print_units(const state* s, const unsigned char* buffer, int count) {
    printf("%s\n", s->display_mode ? "Hexadecimal" : "Decimal");
    printf("%s\n", s->display_mode ? "==========" : "========");
    
    for(int i = 0; i < count; i++) {
        int val = 0;
        memcpy(&val, &buffer[i * s->unit_size], s->unit_size);
        printf(s->display_mode ? hex_formats[s->unit_size-1] : dec_formats[s->unit_size-1], val);
    }
}

// ============================= Menu Functions ==============================

// ============================= 0 =============================

void toggle_debug_mode(state* s) {
    s->debug_mode = !s->debug_mode;
    printf("Debug flag now %s\n", s->debug_mode ? "on" : "off");
}

// ============================= 1 =============================

void set_file_name(state* s) {
    printf("Enter file name: ");
    if(fgets(s->file_name, MAX_FILENAME_SIZE, stdin) != NULL) {
        trim_newline_len(s->file_name);
        if(s->debug_mode) 
            fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

// ============================= 2 =============================

void set_unit_size(state* s) {
    int size;
    printf("Enter unit size(%d, 2, %d): ", MIN_UNIT_SIZE, MAX_UNIT_SIZE);
    if(scanf("%d", &size) != 1) {
        printf("Error: Invalid input\n");
        while(getchar() != '\n'); // Clear buffer
        return;
    }
    while(getchar() != '\n'); // Clear buffer

    if(size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if(s->debug_mode)
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
    } else {
        printf("Error: Invalid unit size\n");
    }
}

// ============================= 3 =============================

void load_into_memory(state* s) {
    error_code err = SUCCESS;
    FILE* file = open_file_read(s, &err);
    if(!file) {
        printf("Error: %d when opening file\n", err);
        return;
    }

    unsigned int location, length;
    printf("Please enter <location> <length>\n");
    if(scanf("%x %u", &location, &length) != 2) {
        printf("Error: Invalid input format\n");
        fclose(file);
        while(getchar() != '\n'); // Clear buffer
        return;
    }
    while(getchar() != '\n'); // Clear buffer

    if(s->debug_mode) {
        fprintf(stderr, "Debug: file_name: %s, location: %x, length: %u\n", 
                s->file_name, location, length);
    }

    fseek(file, location, SEEK_SET);
    s->mem_count = fread(s->mem_buf, s->unit_size, length, file);
    printf("Loaded %zu units into memory\n", s->mem_count);
    
    fclose(file);
}

// ============================= 4 =============================

void toggle_display_mode(state* s) {
    s->display_mode = !s->display_mode;
    printf("Display flag now %s, %s representation\n",
           s->display_mode ? "on" : "off",
           s->display_mode ? "hexadecimal" : "decimal");
}

// ============================= 5 =============================

void file_display(state* s) {
    error_code err = SUCCESS;
    FILE* file = open_file_read(s, &err);
    if(!file) {
        printf("Error: %d when opening file\n", err);
        return;
    }

    unsigned int offset, length;
    printf("Enter file offset and length\n");
    if(scanf("%x %u", &offset, &length) != 2) {
        printf("Error: Invalid input format\n");
        fclose(file);
        while(getchar() != '\n'); // Clear buffer
        return;
    }
    while(getchar() != '\n'); // Clear buffer

    unsigned char buffer[MAX_BUFFER_SIZE];
    fseek(file, offset, SEEK_SET);
    size_t units_read = fread(buffer, s->unit_size, length, file);
    fclose(file);

    print_units(s, buffer, units_read);
}

// ============================= 6==================================

void memory_display(state* s) {
    unsigned int addr, length;
    printf("Enter address and length\n");
    if(scanf("%x %u", &addr, &length) != 2) {
        printf("Error: Invalid input format\n");
        while(getchar() != '\n'); // Clear buffer
        return;
    }
    while(getchar() != '\n'); // Clear buffer

    // Check bounds for length
    if (length > s->mem_count) {
        printf("Error: Length exceeds available memory units\n");
        return;
    }

    // If addr is 0, start from beginning of buffer
    // Otherwise, treat addr as offset within buffer
    unsigned int offset = (addr == 0) ? 0 : addr;
    
    // Check if offset is within bounds
    if (offset >= MAX_BUFFER_SIZE) {
        printf("Error: Address out of bounds\n");
        return;
    }

    // Check if the range we want to read is within bounds
    if (offset + length > MAX_BUFFER_SIZE) {
        printf("Error: Reading past end of buffer\n");
        return;
    }

    print_units(s, s->mem_buf + offset, length);
}

// ============================= 7 =============================

void save_into_file(state* s) {

    if(s->debug_mode) {
        fprintf(stderr, "Debug: mem_buf is at address: %p\n", (void*)s->mem_buf);
    }

    if(strlen(s->file_name) == 0) {
        printf("Error: file name is empty\n");
        return;
    }

    FILE* file = fopen(s->file_name, "r+b");
    if(!file) {
        printf("Error: opening file failed\n");
        return;
    }

    unsigned int source_addr, target_location, length;
    printf("Please enter <source-address> <target-location> <length>\n");
    if(scanf("%x %x %u", &source_addr, &target_location, &length) != 3) {
        printf("Error: Invalid input format\n");
        fclose(file);
        while(getchar() != '\n'); // Clear buffer
        return;
    }
    while(getchar() != '\n'); // Clear buffer

    if(s->debug_mode) {
        fprintf(stderr, "Debug: source_addr: %x, target_location: %x, length: %d\n", 
                source_addr, target_location, length);
    }

    // Check if target location is valid
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if(target_location >= (unsigned int)file_size) {
        printf("Error: Target location beyond file size\n");
        fclose(file);
        return;
    }

    fseek(file, target_location, SEEK_SET);
    const unsigned char* source = (source_addr == 0) ? s->mem_buf : (unsigned char*)source_addr;
    size_t written = fwrite(source, s->unit_size, length, file);
    
    if(written != length) {
        printf("Warning: Only wrote %zu units out of %u\n", written, length);
    }

    fclose(file);
}

// ============================= 8 =============================

void memory_modify(state* s) {
    unsigned int location, val;
    printf("Please enter <location> <val>\n");
    if(scanf("%x %x", &location, &val) != 2) {
        printf("Error: Invalid input format\n");
        while(getchar() != '\n'); // Clear buffer
        return;
    }
    while(getchar() != '\n'); // Clear buffer

    if(s->debug_mode) {
        fprintf(stderr, "Debug: location: %x, val: %x\n", location, val);
    }

    if(location + s->unit_size > MAX_BUFFER_SIZE) {
        printf("Error: Invalid location\n");
        return;
    }

    memcpy(&s->mem_buf[location], &val, s->unit_size);
}

// ============================= 9 =============================

void quit(state* s) {
    if(s->debug_mode) 
        fprintf(stderr, "Debug: quitting\n");
    exit(0);
}

// ============================== Menu Definition ==============================

struct fun_desc menu[] = {
    { "Toggle Debug Mode", toggle_debug_mode },
    { "Set File Name", set_file_name },
    { "Set Unit Size", set_unit_size },
    { "Load Into Memory", load_into_memory },
    { "Toggle Display Mode", toggle_display_mode },
    { "File Display", file_display },
    { "Memory Display", memory_display },
    { "Save Into File", save_into_file },
    { "Memory Modify", memory_modify },
    { "Quit", quit },
    { NULL, NULL }
};

void display_menu(void) {
    printf("\nChoose action:\n");
    for(int i = 0; menu[i].name != NULL; i++) {
        printf("%d-%s\n", i, menu[i].name);
    }
}

void process_inputs(int bounds, state* s) {
    char input[MAX_INPUT_SIZE];
    if (!fgets(input, MAX_INPUT_SIZE, stdin)) {  
        printf("Error reading input\n");
        return;
    }

    int input_len = trim_newline_len(input); 
    int valid = validation(input_len, input);
    int choice = atoi(input);

    if (!valid || choice < 0 || choice >= bounds) {
        printf("Invalid input. Please enter a valid menu option.\n");
        return;
    }

    printf("Option: %d\n", choice);
    menu[choice].fun(s);
    printf("DONE.\n");
}

// ============================== Main Function ==============================

int main(int argc, char **argv) {
    int bounds = (sizeof(menu) / sizeof(menu[0])) - 1; // -1 for NULL terminator

    state s = {
        .debug_mode = 0,
        .unit_size = 1,
        .display_mode = 0,
        .mem_count = 0,
        .file_name = ""
    };

    while(1) {
        if(s.debug_mode) {
            print_debug_info(&s);
        }

        display_menu();
        process_inputs(bounds, &s);
    }

    return 0;
}