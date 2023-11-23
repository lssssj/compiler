#include <stdio.h>
#include <stdint.h>

enum TT {
    OO,
    YY,
    T
};

enum LL {
    KK,
    L
};

extern
    int add(int, int);


typedef int32_t TT_t;
/// #undef type_t
#define type_t enum LL

int main() {

    int x = add(10, 11);
    printf("res = %d\n", x);
    return 0;
}