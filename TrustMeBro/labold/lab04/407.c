#include <stdio.h>

int main() {
    double price, percent;
    int quantity;

    scanf("%lf", &price);
    scanf("%lf", &percent);
    scanf("%d", &quantity);

    double disP = price * quantity * (100 - percent) / 100.0;
    double b2g1 = price * (quantity - (quantity / 3));

    if (disP <= b2g1) {
        printf("Discount %.0lf%%\n", percent);
        printf("%.2lf\n", disP);
    } else {
        printf("Buy 2 Get 1\n");
        printf("%.2lf\n", b2g1);
    }

    return 0;
}