#include <stdio.h>

int main() 
{
    int d;
    scanf("%d", &d);

    for(int i = 2; i < d; i++) {
        if (d % i == 0)
            printf("0");
            return 0;
    }
    printf("1");
    return 0;
}