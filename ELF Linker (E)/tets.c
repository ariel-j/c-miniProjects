#include <stdio.h>
#include <stdlib.h>

#define MAX_LENGTH 1024

void main() {
    char input_buff[MAX_LENGTH];
    int addr1 = 0x40;
    char *addr2 = &addr1;

    fgets(input_buff, MAX_LENGTH, stdin);
    sscanf(input_buff, "%x", addr2);
    printf("%x", addr1);
    printf("%x", *addr2);
}