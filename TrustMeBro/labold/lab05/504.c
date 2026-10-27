#include "stdio.h"

int main() {

    int n;

    scanf("%d", &n);

    int j = 1;
    while (j <= n){
        printf("%d ", j);
        j += 1;
    }

    return 0;
}
