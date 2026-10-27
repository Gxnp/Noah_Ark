#include <stdio.h>

int main() {
    char num[6];
    scanf("%s", num);

    printf("%c%c%c%c%c",num[2],num[3],num[4],num[0],num[1]);

    return 0;
}