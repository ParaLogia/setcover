#ifndef SETCOVER_H
#define SETCOVER_H

void read_sets(char *filename);

void free_sets();

void backtrack(int k);

void print_subset(int i);

int compare_set_sz(const void *set_ptr1, const void *set_ptr2);

void init_suffixes();

void del_trivial_subsets();

#endif