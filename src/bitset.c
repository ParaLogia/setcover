#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "bitset.h"

const size_t int_sz = sizeof(int) << 3;


int *alloc_set(int nbits)
{
    int nwords = (nbits + int_sz - 1) / int_sz;
    int *rtn = calloc(nwords, int_sz);
    if (rtn == NULL)
    {
        perror("Error from calloc");
    }
    return rtn;
}

void add_elem(Bitset set, int n)
{
    set[n/int_sz] |= (1 << (n % int_sz));
}

void del_elem(Bitset set, int n)
{
    set[n/int_sz] &= ~(1 << (n % int_sz));
}

void print_set(Bitset set, int nbits)
{
    for (int i = 0; i < nbits; i++)
    {
        if (set[i/int_sz] & (1 << (i % int_sz)))
            printf("%d ", i+1);
    }
    printf("\n");
}

int set_sz(Bitset set, int nbits)
{
    int count = 0;
    for (int i = 0; i < nbits; i++)
    {
        if (set[i/int_sz] & (1 << (i % int_sz)))
            count++;
    }
    return count;
}

bool is_universal(Bitset set, int nbits)
{
    int nwords = (nbits + int_sz - 1) / int_sz;
    for (int i = 0; i < nwords - 1; i++)
    {
        if (~set[i])
            return false;
    }
    return !~set[nwords-1] || (!~(set[nwords-1] | ~0 << (nbits % int_sz)));
}

bool is_superset(Bitset super, Bitset sub, int nbits)
{
    int nwords = (nbits + int_sz - 1) / int_sz;
    for (int i = 0; i < nwords - 1; i++)
    {
        if ((sub[i] & ~super[i]))
            return false;
    }
    return !(sub[nwords-1] & ~super[nwords-1])
        || ! ( (sub[nwords-1] & ~super[nwords-1]) << (int_sz - (nbits % int_sz)) );
}

void add_all(Bitset set, Bitset add, int nwords)
{
    for (int i = 0; i < nwords; i++)
    {
        set[i] |= add[i];
    }
}

void for_each_elem(Bitset set, unsigned int nbits,
                    intconsumer consumer)
{
    for (int i = 0; i < nbits; i++)
    {
        if (set[i/int_sz] & (1 << (i % int_sz)))
            consumer(i);
    }
}