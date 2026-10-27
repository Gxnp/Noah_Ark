#include <stdio.h>

int main() {
    int start, end;
    double distance = 0, time;

    scanf("%d %d", &start, &end);

    if (start > end) {
        int temp = start;
        start = end;
        end = temp;
    }

    switch (start * 10 + end) {
        case 12: distance = 6; break;
        case 13: distance = 10; break;
        case 14: distance = 14; break;
        case 15: distance = 18; break;
        case 16: distance = 22; break;

        case 23: distance = 4; break;
        case 24: distance = 8; break;
        case 25: distance = 12; break;
        case 26: distance = 16; break;

        case 34: distance = 4; break;
        case 35: distance = 8; break;
        case 36: distance = 12; break;

        case 45: distance = 4; break;
        case 46: distance = 8; break;

        case 56: distance = 4; break;

        default:
            distance = 0;
    }

    if (distance == 0) {
        time = 0;
    } else {
        time = 6 + (distance - 4.5) / 1.5;
    }

    printf("%.2lf\n", time);

    return 0;
}