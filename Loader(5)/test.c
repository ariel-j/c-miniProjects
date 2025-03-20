extern int system_call();

// Our own strlen
int string_length(char* str) {
    int len = 0;
    while(str[len] != '\0') len++;
    return len;
}

int main(int argc, char** argv) {
    system_call(4, 1, "Arguments received:\n", 19);
    
    for(int i = 0; i < argc; i++) {
        system_call(4, 1, "arg[", 4);
        char num = '0' + i;
        system_call(4, 1, &num, 1);
        system_call(4, 1, "]: ", 3);
        system_call(4, 1, argv[i], string_length(argv[i]));
        system_call(4, 1, "\n", 1);
    }
    return 0;
}