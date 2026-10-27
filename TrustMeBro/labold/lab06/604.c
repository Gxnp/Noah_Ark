#include <stdio.h>

int main() {
    char str[101];
    int length = 0;
    char ch;
    
    while (length < 100 && scanf("%c", &ch) == 1 && ch != '\n') {
        str[length++] = ch;
    }
    
    for (int i = length - 1; i >= 0; i--) {
        printf("%c", str[i]);
    }
    printf("\n");
    
    return 0;
}