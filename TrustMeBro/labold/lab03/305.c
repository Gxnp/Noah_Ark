#include <stdio.h>

int main() {
    double a, b;
    scanf("%lf", &a);
    scanf("%lf", &b);

    printf("Perimeter of rectangle = %.4lf units", (a+b)*2);

    return 0;
}