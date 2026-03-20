#include "radix_sort.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

void radix_sort_float(float *arr, size_t n)
{
    if (!arr || n < 2)
        return;

    uint32_t *keys = (uint32_t*)arr;
    uint32_t *tmp = malloc(n * sizeof(uint32_t));
    if (!tmp)
        return;

    for (size_t i = 0; i < n; i++) 
    {
        uint32_t x = keys[i];
        if (x & 0x80000000)
            keys[i] = ~x;
        else
            keys[i] = x ^ 0x80000000;
    }

    for (int shift = 0; shift < 32; shift += 8) 
    {
        size_t count[256] = {0};

        for (size_t i = 0; i < n; i++)
            count[(keys[i] >> shift) & 0xFF]++;

        size_t sum = 0;
        for (int i = 0; i < 256; i++) 
        {
            size_t tmp_sum = count[i];
            count[i] = sum;
            sum += tmp_sum;
        }

        for (size_t i = 0; i < n; i++) 
        {
            uint32_t k = keys[i];
            tmp[count[(k >> shift) & 0xFF]++] = k;
        }

        memcpy(keys, tmp, n * sizeof(uint32_t));
    }

    for (size_t i = 0; i < n; i++) {
        uint32_t x = keys[i];
        if (x & 0x80000000)
            keys[i] = x ^ 0x80000000;
        else
            keys[i] = ~x;
    }

    free(tmp);
}
