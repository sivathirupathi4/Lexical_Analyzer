#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[])
{
    FILE *fp;

    if (argc != 2)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "r");

    if (fp == NULL)
    {
        printf("Error: Unable to open file %s\n", argv[1]);
        return 1;
    }

    lexical_analyzer(fp);

    fclose(fp);

    return 0;
}