#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <stdlib.h>

typedef void (*intconsumer)(int);

typedef int *Bitset;

/* Number of bits in an int */
extern const size_t int_sz;


/**
 * Creates a bit set with space for at least n bits.
 *
 * Must be freed.
 */
int *alloc_set(int nbits);

/**
 * Add element n to the given set. (Elements numbered from 0)
 * In other words, set the bit at index n to 1.
 *
 * No bounds checking is performed.
 */
void add_elem(Bitset set, int n);

/**
 * Remove element n from the given set. (Elements numbered from 0)
 * In other words, set the bit at index n to 0.
 *
 * No bounds checking is performed.
 */
void del_elem(Bitset set, int n);

/* Prints out the elements of a set in a line */
void print_set(Bitset set, int nbits);

/* Returns how many elements are in this set */
int set_sz(Bitset set, int nbits);

/* Tests whether a set covers the universal set of nbits elements */
bool is_universal(Bitset set, int nbits);

/* Tests whether super is a superset of sub */
bool is_superset(Bitset super, Bitset sub, int nbits);

/**
 * Adds all elements of add to set
 * Note that the third argument is in words, not bits
 */
void add_all(Bitset set, Bitset add, int nwords);


/* Performs the given function on each set element */
void for_each_elem(Bitset set, unsigned int nbits, \
                    intconsumer consumer);

#endif