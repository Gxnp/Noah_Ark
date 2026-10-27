#include <stdio.h>

int main() {
    char fname1[25], sname1[25], person2[50], person3[50];

    scanf("%s", fname1);
    scanf("%s", sname1);
    scanf(" %[^\n]",person2);
    scanf(" %[^\n]",person3);

    printf("Person 1: %s %s\n", fname1, sname1);
    printf("Person 2: %s\n", person2);
    printf("Person 3: %s\n", person3);

    return 0;
}