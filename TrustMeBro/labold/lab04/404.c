#include <stdio.h>

int main() {
    double score;

    scanf("%lf", &score);

    if (score < 0 || score > 100)
    {
        printf("Out of Range");
    } else if (score >= 80)
    {
        printf("A");
    } else if (score >= 70)
    {
        printf("B");
    } else if (score >= 60)
    {
        printf("C");
    } else if (score >= 50)
    {
        printf("D");
    } else
    {
        printf("F");
    }

    return 0;
}