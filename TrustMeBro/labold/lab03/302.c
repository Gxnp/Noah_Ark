#include <stdio.h>

int main() {
    double a, b, c, d;
    scanf(" %lf", &a);
    scanf(" %lf", &b);
    scanf(" %lf", &c);
    scanf(" %lf", &d);

    printf("Summation is %.2lf\n",a+b+c+d);
    printf("Average is %.3lf\n", (a+b+c+d)/4);

    return 0;
}