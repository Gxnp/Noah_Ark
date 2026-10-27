#include <stdio.h>

int main()
{
    int a, b;

    scanf("%d", &a);
    scanf("%d", &b);

    printf("%.2lf", (double)a / b);

    return 0;
}