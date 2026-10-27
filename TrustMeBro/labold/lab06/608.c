#include <stdio.h>

int find_index(char text[26], char x){
    for (int j = 0; j < 26; j++) {
        if (x == text[j]) {
            return j;
        }
    }
    return 0;
}

int main(){
    char circle_upper[26] = "ABXYPQRMNCEDKLJOSHTUFVZGWI";
    char circle_lower[26] = "abxypqrmncedkljoshtufvzgwi";
    char message[201];
    
    scanf("%[^\n]", message);
    
    int length = 0;
    while(message[length] != '\0')
        length++;
        
    for (int i = 0; i < length; i++) {
        int index = 0;
        if (message[i] >= 'A' && message[i] <= 'Z'){
            index = find_index(circle_upper, message[i]);
            index = (index + 5) % 26;
            printf("%c", circle_upper[index]);
        } else if (message[i] >= 'a' && message[i] <= 'z'){
            index = find_index(circle_lower, message[i]);
            index = (index + 5) % 26;
            printf("%c", circle_lower[index]);
        } else {
            printf("%c", message[i]);
        }
    }
    printf("\n");
    
    return 0;
}