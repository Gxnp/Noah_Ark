#include <stdio.h>


int main() {
    char s1[30], s2[30], s3[30], s4[30];
    scanf("%s", s1);
    scanf("%s", s2);
    scanf("%s", s3);
    scanf("%s", s4);
    
    printf("String 1: %.3s\nString 2: %.4s\nString 3: %.5s\nString 4: %.6s",s1,s2,s3,s4);

    return 0;
}
