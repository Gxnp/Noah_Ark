#include <stdio.h>

int main(){
    double a,b,c;
    int a1 , b2 , c3;
    scanf("%lf",&a);
    scanf("%lf",&b);
    scanf("%lf",&c);
    a1 = a + b > c;
    b2 = a + c > b;
    c3 = b + c > a;
    if (a1 && b2 && c3)
        printf("Triangle is valid.");
    else
        printf("Triangle is not valid.");
    return 0;
}