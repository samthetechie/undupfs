/*
 * bloom.c: implementation of Bloom filters for undup-fuse
 *
 * Copyright (C) 2013 Andrew Isaacson <adi@hexapodia.org>
 *
 * This program is free software, licensed under the terms of the GNU GPL
 * version 3.  See the file COPYING for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bloom.h"

/*
 * Theory of operation.  This implementation is tuned to the scenario that we
 * have many (1M or more) filters set up with the same parameters.  Therefore
 * it's important that the parameters be stored separately from the bit array,
 * and there's no explicit linkage between them.
 *
 * The parameters are stroed in a `struct bloom_params` which is returned from
 * bloom_setup().  Each filter bitarray is initialized by a call to bloom_init.
 *
 * Keys are assumed to be a hash output, and are assumed to be a random
 * selection from {0,1}^N.  Bits from the key are used directly as inputs to the
 * Bloom filter function, rather than being hashed as in a general purpose Bloom
 * filter implementaiton.
 *
 * Keys are inserted into a filter by calling bloom_insert().
 *
 * Key presence is tested by calling bloom_test().
 */

/*
 * Set up a cohort of Bloom filters.  bloom_setup initializes shared state and
 * validates the settings.
 *
 * @sz: size of the Bloom bitarray, in bits.
 * @nb: number of bits set per key inserted into filter.
 * @kl: length of input keys, in bytes.
 */
struct bloom_params *bloom_setup(int sz, int nb, int kl)
{
    struct bloom_params *p = calloc(sizeof *p, 1);

    if (!p) return p;

    p->size = sz;
    p->bytesize = sz / 8 + !!(sz % 8);
    ASSERT(p->bytesize * 8 >= sz);
    p->nbit = nb;
    p->keylen = kl;

    return p;
}

/*
 * Initialize a single filter given the established settings.
 *
 * @b: bitarray to initialize.  Must be of the appropriate size.
 */
void bloom_init(struct bloom_params *p, u8 *b)
{
    memset(b, 0, p->bytesize);
}


/*
 * Extracts NBIT bits from bit array A starting at bit position POS.
 * Both bits an bytes are indexed big-endian:
 *    \ bitindex|            11 1111
 *  val\        |0123 4567 8901 2345
 *  ------------+-------------------
 *  0x11 0x2f   |0001 0001 0010 1111
 *  extracting:
 *  4 @ 0      = 0001      = 0x01
 *  6 @ 0      = 0001 00   = 0x08
 *  8 @ 0      = 0001 0001 = 0x11
 *  12 @ 0     = 0001 0001 0010 = 0x112
 *  8 @ 4      = 0001 0010 = 0x12
 *  8 @ 2      = 01 0001 00 = 0x44
 *  
 * Values are returned right-justified, so if 4 bits valued 0010 are extracted,
 * the return value is 0x00000002.
 */
static u32 get_bits(u8 *a, int pos, int nbit)
{
    u64 r = 0;
    int i;
    u32 mask = (1LL << nbit) - 1;
    int shift, base = 0;

    ASSERT(nbit <= 32);

    /* pos n shift      input   output
     * --- - -----  --------- --------
     *  0  1  7     1000 0000 00000001
     *  0  2  6     1100 0000 00000011
     *  1  1  6     0100 0000 00000001
     *  2  1  5     0010 0000 00000001
     *  ...
     *  7  1  0     0000 0001 00000001
     *  ...
     *  5  4        0000 0111 00001110
     */
    shift = ((8 - nbit - (pos % 8)) + 4 * 8) % 8;

    if (pos % 8 > 0) {
        r = a[pos/8];
        nbit -= (8 - (pos % 8));
        base = 1;
    }

    for(i = 0; i < nbit; i+= 8) {
        r = (r << 8) | a[(pos + i) / 8 + base];
    }
    return (r >> shift) & mask;
}

int bloom_insert(struct bloom_params *p, u8 *b, u8 *key)
{
    int i;

    for (i=0; i<p->nbit; i++) {
        int b = get_bits(key, i * p->bitperf, p->bitperf);
    }
    return 0;
}

int bloom_test(struct bloom_params *p, u8 *b, u8 *key)
{
    return 0;
}

#ifdef MAIN

int o_verbose = 0;
FILE *f_debug = NULL;

int main(void)
{
    printf("running Bloom filter tests\n");
    printf("testing get_bits ..."); fflush(stdout);
    {
        int ntest = 0;
        u8 a[] = { 0x12, 0x34, 0x55, 0x66, 0x77, 0xff, 0xab };

        ASSERT(get_bits(a, 0, 8)  ==       0x12); ntest++;
        ASSERT(get_bits(a, 0, 4)  ==        0x1); ntest++;
        ASSERT(get_bits(a, 4, 4)  ==        0x2); ntest++;
        ASSERT(get_bits(a, 5, 4)  ==        0x4); ntest++;
        ASSERT(get_bits(a, 5, 8)  ==       0x46); ntest++;
        ASSERT(get_bits(a, 0, 32) == 0x12345566); ntest++;
        ASSERT(get_bits(a, 1, 32) == 0x2468aacc); ntest++;
        ASSERT(get_bits(a, 2, 32) == 0x48d15599); ntest++;
        printf(" passed %d tests\n", ntest);
    }
    return 0;
}
#endif
