#include <stdio.h>
 
int main() {
    int age = 0, height = 0, weight = 0, co1 = 0, co2 = 0, co3 = 0, co4 = 0, avage,co5 = 0;
    float  avheight, avweight, co6 = 0.0, co7 = 0.0;
    for (int i = 0; i < 50 ; i++){
        scanf("%d %d %d", &age, &height, &weight);

        if (age >= 20 && height >= 160)
            co1 += 1;
        if (age < 20 && (height <= 180 || weight >= 60))
            co2 += 1;
        if (age >= 30 && (weight >= 40 && weight <= 80))
            co3 += 1;
        if (age < 40 && (weight < 85 || height <= 200))
            co4 += 1;
        co5 += age;
        co6 += height;
        co7 += weight;
    }

    avage = co5 / 50;
    avheight = co6 / 50;
    avweight = co7 / 50;

    printf("Age >= 20 and Height >= 160: %d\nAge < 20 and Height <= 180 or Weight >= 60: %d\nAge >= 30 and Weight >= 40 and Weight <= 80: %d\nAge < 40 and Weight < 85 or Height <= 200: %d\nAverage Age: %d\nAverage Height: %.2f\nAverage Weight: %.2f",co1,co2,co3,co4,avage,avheight,avweight);
    return 0;
}