#ifndef CORE_TYPES_H
#define CORE_TYPES_H

//Try C99 <stdint.h> first//
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* Compiler supports C99 → use real stdint.h */

    #include <stdint.h>

#else
/*Fallback: C89-compatible 
typedefsMust match stdint.h names exactly!*/

    /* 8-bit */
    typedef signed char        int8_t;
    typedef unsigned char      uint8_t;

    /* 16-bit */
    typedef signed short       int16_t;
    typedef unsigned short     uint16_t;

    /* 32-bit (C89 long is at least 32-bit) */
    typedef signed long        int32_t;
    typedef unsigned long      uint32_t;

    /* No 64-bit support in strict C89 */
#endif

#endif /* CORE_TYPES_H */
