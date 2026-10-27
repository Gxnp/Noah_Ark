#include <stdio.h>

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    
    char chars[200];
    int counts[200] = {0};
    int unique_count = 0;
    
    for (int i = 0; i < n; i++) {
        char ch;
        scanf(" %c", &ch);
        
        if (ch >= 'A' && ch <= 'Z') {
            ch += 32;
        }
        
        int found = -1;
        for (int j = 0; j < unique_count; j++) {
            if (chars[j] == ch) {
                found = j;
                break;
            }
        }
        
        if (found != -1) {
            counts[found]++;
        } else {
            chars[unique_count] = ch;
            counts[unique_count] = 1;
            unique_count++;
        }
    }
    
    for (int i = 0; i < unique_count; i++) {
        printf("%c: %d\n", chars[i], counts[i]);
    }
    
    return 0;
}