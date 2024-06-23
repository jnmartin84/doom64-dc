
/* m_fixed.c -- fixed point implementation */

#include "i_main.h"
#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"

fixed_t D_abs(fixed_t x)
{
    fixed_t _s = x >> 31;
    return (x ^ _s) - _s;
}
#if 0
fixed_t finesine(int x)
{
#if 0
    // original has qA = 12 (output range [-4095,4095])
    // we need 16 (output range [-65535,65535])
//    static const int qN = 13, qA= 16, qP= 15, qR= 2*qN-qP, qS= qN+qP+1-qA;

    // scale the input range
    // this makes it work as a replacement for the old finesine table
//    x <<= 2;
//    x = (x << (30 - qN));

    x = x << 19;

    if ((x ^ (x << 1)) < 0)
        x = (1 << 31) - x;

//    x = x >> (30 - qN);
    x = x >> 17;

//    return x * ((3 << qP) - ((x * x) >> qR)) >> qS;
    return x * (98304 - ((x * x) >> 11)) >> 13;
#endif
return tfinesine[x];
}

fixed_t finecosine(int x) {
    return finesine(x + 2048);
}

angle_t tantoangle(int x) {
//    return ((angle_t)((-47*((x)*(x))) + (359628*(x)) - 3150270));
return ttantoangle[x];
}
#endif
/*
===============
=
= FixedDiv
=
===============
*/
fixed_t FixedDiv(fixed_t a, fixed_t b) // 80002BF8
{
    fixed_t     aa, bb;
    unsigned    c;
    int         sign;

    sign = a^b;

    if (a < 0)
        aa = -a;
    else
        aa = a;

    if (b < 0)
        bb = -b;
    else
        bb = b;

    if ((unsigned)(aa >> 14) >= bb)
    {
        if (sign < 0)
            c = MININT;
        else
            c = MAXINT;
    }
    else
    {
        c = (fixed_t) FixedDiv2(a, b);
    }

    return c;
}

/*
===============
=
= FixedDiv2
=
===============
*/
fixed_t FixedDiv2(register fixed_t a, register fixed_t b)//L8003EEF0()
{
    s64 result = ((s64) a << 16) / (s64)b;

    return (fixed_t) result;
}

/*
===============
=
= FixedMul
=
===============
*/
fixed_t FixedMul(fixed_t a, fixed_t b) // 800044D0
{
    s64 result = ((s64) a * (s64) b) >> 16;

    return (fixed_t) result;
}
