#include "stdio.h"

int main() {

    int start;

    scanf("%d", &start);

    for (int i = 1; i <= start; i++) {
        for (int j = 0; j < start-i; j++){
            printf(" ");
        }
        for (int k = 0; k < 2*i-1; k++){
            printf("*");
        }
        printf("\n");
    }
    return 0;
}
