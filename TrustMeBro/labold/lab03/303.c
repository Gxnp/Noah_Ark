#include <stdio.h>
#include <math.h>

int main() {
    double a, b, c;
    scanf(" %lf", &a);
    scanf(" %lf", &b);

    c = pow((pow(a,2)+(pow(b,2))),0.5);

    printf("%.2lf", c);

    return 0;
}