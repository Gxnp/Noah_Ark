#include <stdio.h>

int main() {
    char str[301];
    int length = 0;
    char ch;
    
    while (length < 300 && scanf("%c", &ch) == 1 && ch != '\n') {
        str[length++] = ch;
    }
    str[length] = '\0';
    
    scanf(" %c", &ch);
    
    char target_lower = ch;
    char target_upper = ch;
    if (ch >= 'A' && ch <= 'Z') {
        target_lower = ch + 32;
    } else if (ch >= 'a' && ch <= 'z') {
        target_upper = ch - 32;
    }
    
    int count = 0;
    int positions[300];
    
    for (int i = 0; i < length; i++) {
        if (str[i] == ch || str[i] == target_lower || str[i] == target_upper) {
            positions[count] = i + 1;
            count++;
        }
    }
    
    if (count == 0) {
        printf("Not found.\n");
    } else {
        printf("There is/are %d \"%c\" in the above sentences.\n", count, ch);
        printf("Position:");
        for (int i = 0; i < count; i++) {
            printf(" %d", positions[i]);
            if (i < count - 1) {
                printf(",");
            }
        }
        printf("\n");
    }
    
    return 0;
}