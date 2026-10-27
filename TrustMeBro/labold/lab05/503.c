#include "stdio.h"

int main() {

    int start, end;

    scanf("%d %d", &start, &end);

    if (start < end) {
        for (int i = start; i <= end; i++) {
            printf("%d ", i);
        }
    } else {
        for (int i = start; i >= end; i--) {
            printf("%d ", i);
        }
    }

    return 0;
}
