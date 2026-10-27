#include <stdio.h>

int main() {
    long a;
    scanf("%ld", &a);

    int day, hour, minute, second;

    day = a / 86400;
    hour = (a % 86400) / 3600;
    minute = (a % 86400) % 3600 / 60;
    second = (a % 86400) % 3600 % 60;

    printf("%ld s = %d d %d h %d m %d s",a,day,hour,minute,second);
    return 0;
}