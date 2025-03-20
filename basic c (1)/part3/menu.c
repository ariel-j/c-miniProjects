#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char my_get(char c) {
    return fgetc(stdin);
}

char cprt(char c) {
    if (c >= 0x20 && c <= 0x7E)     {
        printf("%c\n", c);
    }
    else     {
        printf(".\n");
    }
    return c;
}

char encrypt(char c) {
    if (c >= 0x21 && c <= 0x7F) {
        return c + 1;
    }
    return c;
}

char decrypt(char c) {
    if (c >= 0x21 && c <= 0x7F) {
        return c - 1;
    }
    return c;
}

char xprt(char c) {
    printf("%x\n", c);
    return c;
}

char dprt(char c) {
    printf("%d\n", c);
    return c;
}

struct fun_desc {
    char *name;
    char (*fun)(char);
};

char *map(char *array, int array_length, char (*f)(char)) {
    char *mapped_array = malloc(array_length * sizeof(char));
    if (!mapped_array)     {
        perror("Failed to alocate memory");
    }
    for (int i = 0; i < array_length; i++)     {
        mapped_array[i] = f(array[i]);
    }
    return mapped_array;
}

struct fun_desc menu[] = {
    {"Get String", my_get},
    {"Print Decimal (dprt)", dprt},
    {"Print Hex (xprt)", xprt},
    {"Print Character (cprt)", cprt},
    {"Encrypt", encrypt},
    {"Decrypt", decrypt},
    {NULL, NULL}};

char *map(char *array, int array_length, char (*f)(char));

void print_menu(struct fun_desc *menu)
{
    printf("Select a function from 0-5:\n");
    for (int i = 0; menu[i].name != NULL; i++)
    {
        printf("%d) %s\n", i, menu[i].name);
    }
}

char *input_processing(char *carray, int size, int bounds)
{
    char buffer[100];
    if (!fgets(buffer, sizeof(buffer), stdin))
    {
        printf("\n EOF, exiting program.\n");
        exit(1);
    }
    int user_choise = atoi(buffer);
    if (user_choise < 0 || user_choise >= bounds)
    {
        printf("Not within bounds, exiting...\n");
        exit(1);
    }
    printf("Within bounds\n");
    char *ans = map(carray, size, menu[user_choise].fun);
    free(carray);
    printf("done \n");
    return ans;
}

int main(int argc, char **argv)
{
    int array_size = 5;
    char *carray = malloc(array_size * sizeof(char));
    if (carray == NULL)
    {
        perror("Failed to malloc memory");
    }
    for (int i = 0; i < array_size; i++)
    {
        carray[i] = '\0';
    }
    int bounds = (sizeof(menu) / sizeof(menu[0])) - 1;
    while (1)
    {
        print_menu(menu);
        carray = input_processing(carray, array_size, bounds);
    }
    free(carray);
    return 0;
}