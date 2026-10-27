#include <stdio.h>

int main() {
    float balance, cash, amount;
    int error = 0;
    char protocol;

    scanf("%f %f", &balance, &cash);

    while (error < 3) {
        scanf(" %c", &protocol);

        if (protocol == 'E') {
            scanf(" %c", &protocol);
            break;
        }

        scanf("%f", &amount);

        if ((protocol == 'D' && cash >= amount) ||
            (protocol == 'W' && balance >= amount)) {

            if (protocol == 'D') {
                balance += amount;
                cash -= amount;
            } else {
                balance -= amount;
                cash += amount;
            }
            error = 0;
        } else {
            error++;
        }
    }

    printf("%.2f\n%.2f", balance, cash);
}