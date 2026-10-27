#include <stdio.h>
 
int main() {
    int start = 1, mid, stop, m ,n;
    scanf("%d", &n);

    stop = n;
 
    for (m = 1; m <= n; m++){
        int i = 1 , j = n;
        for(i; i <=j; i++) {
            if (i == start || i == stop) {
                printf("-");
            }
            else {
                printf("#");
            }
        }
        printf("\n");
        start += 1;
        stop -= 1;
    }
 
    return 0;
}