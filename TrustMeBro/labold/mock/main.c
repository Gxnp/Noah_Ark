#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int cmp(const void *a,const void *b){
    double x = *(double *)a;
    double y = *(double *)b;

    // น้อย -> มาก
    return (x>y) - (x<y); // >-< M:N
    // มาก -> น้อย
    // return (x<y) - (x>y); // <-> ก็ต่อเมื่อ
}

int main(){
    int num;
    scanf("%d", &num);

    double list[num];
    int i;
    double sum = 0.0;
    for (i = 0; i<num; i++){
        scanf("%lf", &list[i]);
        sum += list[i];
    }

    double sd,zigma;
    // double zigma;
    sd = 0.0;
    zigma = 0.0;
    for (i = 0 ; i<num; i++){
        zigma += ((list[i] - sum/num)*(list[i] - sum/num));
    }
    sd = sqrt(zigma/num);

    qsort(list,num,sizeof(double),cmp);

    printf("%.2lf\n",sum/num);
    if (num%2){
        printf("%.2lf\n", list[(num)/2]);
    }
    else{
        //1 2 3 4 5 6 7 8 9 10
        printf("%.2lf\n", (list[(num/2)] + list[(num/2)-1])/2);
    }
    printf("%.2lf\n",sd);

    return 0;
}
