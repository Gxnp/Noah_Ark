#include <stdio.h>

int main() {
    double a, b, c, volume;
    double pii = 3.14159265359;
    scanf("%lf", &a);
    scanf("%lf", &b);
    scanf("%lf", &c);

    volume = pii * ((b/2) * (b/2)) * c;

    printf("Volume : %.2lfml\n", volume);
    printf("Baht/ml : %.4lf\n", a/volume);

    return 0;
}