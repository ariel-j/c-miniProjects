#include <stdio.h>

int main() {
    FILE *f = fopen("testA", "wb");
    if (!f) {
        perror("Failed to create file");
        return 1;
    }

    unsigned char data[] = {
        0x56, 0x49, 0x52, 0x42,  // Magic number
        0x00, 0x05,              // Signature size (5 bytes)
        0x56, 0x49, 0x52, 0x55, 0x53,  // Signature "VIRUS"
        0x00, 0x00, 0x00, 0x00, 0x00,  // Padding
        0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35 // Rest
    };

    fwrite(data, sizeof(data), 1, f);
    fclose(f);

    printf("Binary file 'testA' created successfully.\n");
    return 0;
}
