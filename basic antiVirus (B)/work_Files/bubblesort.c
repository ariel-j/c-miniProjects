#include <stdio.h>
#include <stdlib.h>

void bubbleSort(int numbers[], int array_size)
{
    int i, j, temp;  
    for (i = (array_size - 1); i > 0; i--)
    {
        for (j = 1; j <= i; j++)  
        {
            if (numbers[j - 1] > numbers[j])
            {
                temp = numbers[j - 1];
                numbers[j - 1] = numbers[j];
                numbers[j] = temp;
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc <= 1)  
    {
        printf("Usage: %s <list of integers>\n", argv[0]);
        return 1;
    }

    char **arr = argv + 1;
    int i, n = argc - 1;
    int *numbers = (int *)calloc(n, sizeof(int));  
    if (!numbers)      {
        perror("Failed to allocate memory");
        return 1;
    }

    printf("Original array:");
    for (i = 0; i < n; ++i)  
    {
        printf(" %s", arr[i]);
        numbers[i] = atoi(arr[i]);
    }
    printf("\n");

    bubbleSort(numbers, n);

    printf("Sorted array:");
    for (i = 0; i < n; ++i)
        printf(" %d", numbers[i]);
    printf("\n");

    free(numbers);  // Free dynamically allocated memory
    return 0;
}
