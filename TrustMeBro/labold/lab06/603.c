#include <stdio.h>
#include <stdbool.h>

int main() {
    double matrix[3][3];
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            scanf("%lf", &matrix[i][j]);
        }
    }
    
    bool isScalar = true;
    double diagValue = matrix[0][0]; 
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j) {
                if (matrix[i][j] != diagValue) {
                    isScalar = false;
                }
            } else {
                if (matrix[i][j] != 0.0) {
                    isScalar = false;
                }
            }
        }
    }
    
    if (isScalar) {
        printf("This is a scalar matrix\n");
    } else {
        printf("This is not a scalar matrix\n");
    }
    
    return 0;
}