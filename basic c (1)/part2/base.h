// base.h
#ifndef BASE_H
#define BASE_H

// Declare the functions that you want to import
char my_get(char c);
char cprt(char c);
char encrypt(char c);
char decrypt(char c);
char xprt(char c);
char dprt(char c);
char* map(char *array, int array_length, char (*f)(char));

#endif // BASE_H
    