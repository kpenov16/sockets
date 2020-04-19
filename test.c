#include <stdlib.h>
#include <stdio.h>

void main(void){
    int n = sizeof(unsigned int);
    printf("%d\n", n);

    int unsigned_char = sizeof(unsigned char);
    printf("unsigned_char has %d bytes\n", unsigned_char);

    int unsigned_short = sizeof(unsigned short);
    printf("unsigned_short has %d bytes\n", unsigned_short);

    unsigned int unsigned_int = sizeof(unsigned int);
    printf("unsigned_int has %d bytes\n", unsigned_int);
}