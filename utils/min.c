#include <stdint.h>
#include "min.h"
int32_t min_int32(int32_t a, int32_t b){
    if(a > b){
        return b;
    }
    return a;
}


uint32_t min_uint32(uint32_t a, uint32_t b){
    if(a > b){
        return b;
    }
    return a;
}