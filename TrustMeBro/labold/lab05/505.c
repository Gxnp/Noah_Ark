#include "stdio.h"

int main() {

    int start;

    scanf("%d", &start);

    if (start < 0) {
        for (int i = start; i <= 0; i++) {
            printf("%d ", i);
        }
    } else {
        for (int i = start; i >= 0; i--) {
            printf("%d ", i);
        }
    }

    return 0;
}
