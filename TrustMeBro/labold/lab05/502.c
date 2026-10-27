#include "stdio.h"

int main() {

    int n;

    scanf("%d", &n);

    do {
        printf("%d ", n);
        n -= 1;
    } while (n > 0);

    return 0;
}
