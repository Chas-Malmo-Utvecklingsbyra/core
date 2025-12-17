#include <stdio.h>
#include <stdint.h>
#include "../utils/clock_monotonic.h"

int main(void)
{
    uint64_t test1 = SystemMonotonicMS();
    uint64_t test2 = SystemMonotonicMS();

    /* Time should be greater than zero */
    if (test1 == 0)
        return 1;

    /* Time should not go backwards */
    if (test2 < test1)
        return 1;

    printf("Clock monotonic test passed\n");
    return 0;
}
