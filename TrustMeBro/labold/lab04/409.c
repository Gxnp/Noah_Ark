#include <stdio.h>

int main(){
    double dest;
    scanf("%lf",&dest);
    if (dest>=0 && dest <= 5.032){
        printf("Bangkok");
    }else if (dest > 5.032 && dest <= 35.477){
        printf("Samut Prakarn");
    }else if ( dest > 35.477 && dest <= 52.900){
        printf("Chachoengsao");
    }else if (dest > 52.900 && dest <= 58.855){
        printf("Chon Buri");
    }else{
        printf("InValid");
    }
    return 0;
}