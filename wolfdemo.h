#ifndef __WOLFDEMO_H
#define __WOLFDEMO_H

typedef int32_t fixed_t;
typedef uint8_t boolean_t;
#define false 0
#define true  1

#define UNSIGNED_DIFF( a, b ) ( (a) < (b) ? (b) - (a) : (a) - (b) )

#if 0
// integer square root

#define step(shift) \
    if((0x40000000l >> shift) + root <= value)          \
    {                                                   \
        value -= (0x40000000l >> shift) + root;         \
        root = (root >> 1) | (0x40000000l >> shift);    \
    }                                                   \
    else                                                \
    {                                                   \
        root = root >> 1;                               \
    }

static int32_t iSqrt(int32_t value)
{
    int32_t root = 0;

    step( 0);
    step( 2);
    step( 4);
    step( 6);
    step( 8);
    step(10);
    step(12);
    step(14);
    step(16);
    step(18);
    step(20);
    step(22);
    step(24);
    step(26);
    step(28);
    step(30);

    // round to the nearest integer, cuts max error in half
    if(root < value)
    {
        ++root;
    }

    return root;
}
#else
static uint32_t approxDist(int32_t dx, int32_t dy)
{
    uint32_t min, max;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    if (dx < dy )
    {
        min = dx;
        max = dy;
    }
    else
    {
        min = dy;
        max = dx;
    }

    // coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
    return ((max << 8) + (max << 3) - (max << 4) - (max << 1) +
             (min << 7) - (min << 5) + (min << 3) - (min << 1)) >> 8;
}
#endif

// 16.16 fixed point math functions

#define I2F(a) ((a) << 16)
#define F2I(a) ((a) >> 16)
#define FIX_SIN(a) ((a) < 0 ? sintab[(a + 411775) >> 10] : (a) > 411774 ? sintab[(a - 411775) >> 10] : sintab[(a) >> 10])
#define FIX_COS(a) ((a) < 0 ? costab[(a + 411775) >> 10] : (a) > 411774 ? costab[(a - 411775) >> 10] : costab[(a) >> 10])
#define FIX_OOS(a) ((a) < 0 ? oostab[(a + 411775) >> 10] : (a) > 411774 ? oostab[(a - 411775) >> 10] : oostab[(a) >> 10])
#define FIX_OOC(a) ((a) < 0 ? ooctab[(a + 411775) >> 10] : (a) > 411774 ? ooctab[(a - 411775) >> 10] : ooctab[(a) >> 10])

#if 0
static fixed_t FIX_MUL( fixed_t a, fixed_t b )
{
    int64_t c = (int64_t)a * b;
    return (fixed_t)(c >> 16);
}
#else
static fixed_t FIX_MUL( fixed_t a, fixed_t b )
{
    fixed_t res = 0, c = 0, d = 0, e = 0;
    asm volatile (
        "tst.l %1\n\t"
        "spl %5\n\t"
        "bpl.b 1f\n\t"
        "neg.l %1\n"
        "1:\n\t"
        "tst.l %2\n\t"
        "bpl.b 2f\n\t"
        "not.b %5\n\t"
        "neg.l %2\n"
        "2:\n\t"
        "move.w %1,%3\n\t"
        "swap %1\n\t"
        "move.w %2,%4\n\t"
        "move.w %2,%0\n\t"
        "swap %2\n\t"
        "mulu %3,%0\n\t"
        "mulu %1,%4\n\t"
        "mulu %2,%1\n\t"
        "mulu %3,%2\n\t"
        "swap %1\n\t"
        "move.w #0,%1\n\t"
        "move.w #0,%0\n\t"
        "swap %0\n\t"
        "add.l %4,%0\n\t"
        "addx.l %2,%0\n\t"
        "addx.l %1,%0\n\t"
        "tst.b %5\n\t"
        "bne.b 3f\n\t"
        "neg.l %0\n"
        "3:\n\t"
        : "=d" (res), "=d" (a), "=d" (b), "=d" (c), "=d" (d), "=d" (e)
        : "0" (res), "1" (a), "2" (b), "3" (c), "4" (d), "5" (e)
        : "cc"
    );
    return(res);
}
#endif

#define BLOCK_SIZE      32
#define BLOCK_SHIFT     5

#define HIT_WIDTH       5

#define MAP_WIDTH       24
#define MAP_HEIGHT      24

#define VISPLANEDIST_TIMES_WALLHEIGHT 2000

// Holds the result of a ray being cast.
typedef struct
{
    uint8_t targetBlockX;
    uint8_t targetBlockY;
    uint8_t textureId;
    uint8_t sliceHeight;
    uint16_t textureOffset;
} SLICE;

#endif //__WOLFDEMO_H
