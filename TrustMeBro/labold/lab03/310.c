#include <stdio.h>

int main() {
    float tall;
    float kg;
    scanf("%f", &tall);
    scanf("%f", &kg);

    printf("%.6f", kg/((tall/100)*(tall/100)));

    return 0;
}