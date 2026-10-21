#include <stdio.h>

#define MAX 100

int main()
{
    int a = 10;
    int b = 20;
    float result = 10.5;

    if (a < b)
    {
        printf("a is smaller\n");
    }

    // This is a single line comment

    /*
       This is a
       multi-line comment
    */

    a++;
    b += 5;

    if (a == b && a != 0)
    {
        printf("Equal\n");
    }
    return 0;
}