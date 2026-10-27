#include <stdio.h>

int main() {
    char c1, c2, c3, c4, c5;
    scanf(" %c", &c1);
    scanf(" %c", &c2);
    scanf(" %c", &c3);
    scanf(" %c", &c4);
    scanf(" %c", &c5);

    printf("%c\n%c\n%c\n%c\n%c",c1+1,c2,c3+1,c4,c5+1);

    return 0;
}