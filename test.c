#include <stdio.h>
#include "game/tetris.h"

int main()
{
    FILE *fptr;
    char name[30], level[10], score[20]; 
    char buffer[100];
    // Open a file in read mode
    fptr = fopen("toplist.txt", "r");

    while (fgets(buffer, 100, fptr) != NULL)
            printf("                        %s", buffer);

    // Close the file
    fclose(fptr);

    return 0;
}
