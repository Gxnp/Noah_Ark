#include <stdio.h>

int main() {
    char names[20][61];
    
    for (int i = 0; i < 20; i++) {
        int j = 0;
        char ch;
        while (scanf("%c", &ch) == 1 && ch != '\n') {
            if (j < 60) {
                names[i][j++] = ch;
            }
        }
        names[i][j] = '\0';
    }
    
    for (int i = 0; i < 20; i++) {
        int capitalize_next = 1;
        for (int j = 0; names[i][j] != '\0'; j++) {
            char ch = names[i][j];
            if (ch == ' ' || ch == '\t') {
                capitalize_next = 1;
            } else {
                if (capitalize_next) {
                    if (ch >= 'a' && ch <= 'z') {
                        ch -= 32;
                    }
                    capitalize_next = 0;
                } else {
                    if (ch >= 'A' && ch <= 'Z') {
                        ch += 32;
                    }
                }
            }
            names[i][j] = ch;
        }
    }
    
    for (int i = 0; i < 19; i++) {
        for (int j = i + 1; j < 20; j++) {
            int k = 0;
            int swap = 0;
            while (names[i][k] != '\0' && names[j][k] != '\0') {
                if (names[i][k] > names[j][k]) {
                    swap = 1;
                    break;
                } else if (names[i][k] < names[j][k]) {
                    break;
                }
                k++;
            }
            if (!swap && names[i][k] != '\0' && names[j][k] == '\0') {
                swap = 1;
            }
            
            if (swap) {
                char temp[61];
                int m = 0;
                while (names[i][m] != '\0') {
                    temp[m] = names[i][m];
                    m++;
                }
                temp[m] = '\0';
                
                m = 0;
                while (names[j][m] != '\0') {
                    names[i][m] = names[j][m];
                    m++;
                }
                names[i][m] = '\0';
                
                m = 0;
                while (temp[m] != '\0') {
                    names[j][m] = temp[m];
                    m++;
                }
                names[j][m] = '\0';
            }
        }
    }
    
    for (int i = 0; i < 20; i++) {
        printf("%s\n", names[i]);
    }
    
    return 0;
}