#include <stdio.h>

int main() {
    char str[101];
    int length = 0;
    char ch;
    
    while (length < 100 && scanf("%c", &ch) == 1 && ch != '\n') {
        str[length++] = ch;
    }
    str[length] = '\0';
    
    int is_palindrome = 1;
    for (int i = 0; i < length / 2; i++) {
        if (str[i] != str[length - 1 - i]) {
            is_palindrome = 0;
            break;
        }
    }
    
    if (is_palindrome) {
        printf("It is Palindrome.\n");
    } else {
        printf("It is not Palindrome.\n");
    }
    
    return 0;
}