#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base.h"


char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
    if (mapped_array == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < array_length; i++)
    {
        mapped_array[i] = f(array[i]);
    }

    return mapped_array;
}

char print_ascii_value(char c)
{
    printf("%d\n", (int)c);
    return c;
}

/* 1. my_get: Ignores input char c, reads and returns a character from stdin using fgetc */
char my_get(char c)
{
    return fgetc(stdin);
}

/* 2. cprt: Prints ASCII character if it's printable; otherwise prints '.' */
char cprt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
    { // Printable ASCII range
        printf("%c\n", c);
    }
    else
    {
        printf(".\n");
    }
    return c;
}

/* 3. encrypt: Adds 1 to the character if it's between 0x1F and 0x7E */
char encrypt(char c)
{
    if (c >= 0x1F && c <= 0x7E)
    {
        return c + 1;
    }
    return c;
}

/* 4. decrypt: Subtracts 1 from the character if it's between 0x21 and 0x7F */
char decrypt(char c)
{
    if (c >= 0x21 && c <= 0x7F)
    {
        return c - 1;
    }
    return c;
}

/* 5. xprt: Prints the hexadecimal representation of the character */
char xprt(char c)
{
    printf("%02X\n", (unsigned char)c);
    return c;
}

/* 6. dprt: Prints the decimal representation of the character */
char dprt(char c)
{
    printf("%d\n", (unsigned char)c);
    return c;
}

int main(int argc, char **argv)
{
    char arr1[] = {'H', 'E', 'Y', '!'};
    int length = sizeof(arr1) / sizeof(arr1[0]);
    char *arr2 = map(arr1, length, print_ascii_value);
    free(arr2);

    int base_len = 5;   
    char arr[base_len]; 

    // Reading characters using my_get
    printf("Enter %d characters:\n", base_len);
    char *arr6 = map(arr, base_len, my_get);

    // Printing the characters using dprt (decimal representation)
    printf("Decimal representation of input:\n");
    char *arr3 = map(arr6, base_len, dprt);

    // Printing the characters using xprt (hexadecimal representation)
    printf("Hexadecimal representation of input:\n");
    char *arr4 = map(arr3, base_len, xprt);

    // Encrypting the characters
    printf("Encrypted characters:\n");
    char *arr5 = map(arr4, base_len, encrypt);

    // Print encrypted characters using cprt
    printf("Encrypted characters as ASCII (if printable):\n");
    map(arr5, base_len, cprt);


    free(arr2);
    free(arr6);
    free(arr3);
    free(arr4);
    free(arr5);

    return 0;
}
