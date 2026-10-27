#include <stdio.h>

int main() {
    double a, b, c;
    scanf("%lf", &a);
    scanf("%lf", &b);
    scanf("%lf", &c);

    printf("%.2lf\n", (a*c) - (a*c)*(b/100));

    return 0;
}