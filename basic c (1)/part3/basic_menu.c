#include <stdio.h>
#include <stdlib.h>

int main()
{
    char buffer[100];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        printf("Select function from the menu:\n");
        printf("You entered: %s", buffer);
    }
    printf("\n EOF, exiting program.\n");

    return 0;
}
// gcc -m32 -g -Wall -Wno-uninitialized -o menu  menu.c













//  while (1)
//     {
//         printf("Select function from the menu:\n");
//         if (fgets(buffer, sizeof(buffer), stdin) == NULL)
//         {
//             if (feof(stdin))
//             {
//                 printf("\n EOF, exiting program.\n");
//                 break;
//             }
//             else
//             {
//                 perror("Error reading input");
//                 break;
//             }
//         }

//         printf("You selected: %s", buffer);
//     }
