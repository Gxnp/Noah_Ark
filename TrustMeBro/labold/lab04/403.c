#include <stdio.h>

int main() {
    char grade;

    scanf("%c", &grade);

    switch (grade) {
        case 'A':
        case 'a':
            printf("Genius");
            break;

        case 'B':
        case 'b':
            printf("Good");
            break;

        case 'C':
        case 'c':
            printf("Try Harder");
            break;

        case 'D':
        case 'd':
            printf("Very Bad");
            break;

        case 'F':
        case 'f':
            printf("Fail");
            break;

        default:
            printf("Invalid Input");
    }

    return 0;
}