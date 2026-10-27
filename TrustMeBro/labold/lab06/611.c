#include <stdio.h>
#include <math.h>

int main(){
    int size_x;
    int size_y;
    int diff;
    scanf("A%d", &size_x);
    scanf(" A%d", &size_y);
    diff = size_y - size_x;
    if (diff < 0)
        diff *= -1;
    printf("%.0lf\n", pow(2.0, diff));
    return 0;
}