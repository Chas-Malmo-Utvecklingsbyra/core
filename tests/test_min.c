#include <stdio.h>
#include "../utils/min.h"

int main(void)
{
    /* Test min_int32 */
    if (min_int32(5, 3) != 3)
        return 1;

    if (min_int32(-2, 4) != -2)
        return 1;

    /* Test min_uint32 */
    if (min_uint32(10, 20) != 10)
        return 1;

    if (min_uint32(7, 7) != 7)
        return 1;

    printf("All min tests passed\n");
    return 0;
}
