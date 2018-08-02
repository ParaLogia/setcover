#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "setcover.h"
#include "bitset.h"

// Size of the universal set (in bits, bytes, and words)
int usize;
int usize_byte;
int usize_word;

// Number of subsets included (in bits, bytes, and words)
int nsubsets;
int nsubsets_byte;
int nsubsets_word;

// Set of all subsets
Bitset *subsets;

// Bitset at index i is the covers formed by
// taking the last (nsubsets - i) subsets
Bitset *suffix_covers;

/* Global variables to change during backtracking: */

// Smallest set cover thus far, and the corresponding # of subsets
Bitset solution;
int minimum;

// Cover of the universal set by the current selection
Bitset set_cover;

// Current selection of subsets, and corresponding # of subsets
Bitset selection;
int select_sz = 0;

int main(int argc, char *argv[])
{
    // Limit one minute
    // printf("Begin timer: 60 seconds\n");
    alarm(60);

    if (argc < 2)
    {
        printf("Usage: %s FILENAME\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        // Initialize sets by reading from file
        read_sets(argv[1]);
    }

    // Sort subsets by decreasing size
    qsort(subsets, nsubsets, sizeof(Bitset), &compare_set_sz);

    // Delete subsets that are strict subsets of other subsets
    del_trivial_subsets();

    // for (int i = 0; i < nsubsets; i++)
    // {
    //     print_set(subsets[i], usize);
    // }

    init_suffixes();

    // Run our algorithm
    backtrack(0);


    // printf("usize: %d, %d, %d\n", usize, usize_byte, usize_word);
    // printf("nsubsets: %d, %d, %d\n", nsubsets, nsubsets_byte, nsubsets_word);

    // for (int i = 0; i < nsubsets; i++)
    // {
    //     print_set(subsets[i], usize);
    // }

    // Print results
    printf("Minimum: %d\n", minimum);
    for_each_elem(solution, usize, &print_subset);


    // Free all
    free_sets();
}

void backtrack(int k)
{
    if (is_universal(set_cover, usize))
    {
        minimum = select_sz;
        memcpy(solution, selection, nsubsets_byte);
        return;
    }
    // Don't continue if we can't do better than minimum
    if (select_sz >= minimum - 1 || k == nsubsets)
        return;


    // Try adding subset k
    // Save the current cover locally
    int temp_cover[usize];
    memcpy(temp_cover, set_cover, usize_byte);

    // Test if a full cover is possible with the suffix
    add_all(temp_cover, suffix_covers[k], usize_word);
    if (!is_universal(temp_cover, usize))
        return;

    if (!is_superset(set_cover, subsets[k], usize)) {
        memcpy(temp_cover, set_cover, usize_byte);

        // Add subset k
        add_elem(selection, k);
        select_sz++;
        add_all(set_cover, subsets[k], usize_word);

        backtrack(k+1);

        // Undo adding subset k
        memcpy(set_cover, temp_cover, usize_byte);
        select_sz--;
        del_elem(selection, k);
    }

    // Try without adding subset k
    backtrack(k+1);
}

void init_suffixes()
{
    suffix_covers = malloc(nsubsets * sizeof(Bitset));

    int buf[usize_word];
    memset(buf, 0, usize_byte);
    Bitset prev_cover = buf;

    for (int i = nsubsets - 1; i >= 0; i--)
    {
        Bitset suffix_cover = alloc_set(usize);
        memcpy(suffix_cover, prev_cover, usize_byte);
        add_all(suffix_cover, subsets[i], usize_word);
        suffix_covers[i] = suffix_cover;

        // printf("sc[%d] = ", i);
        // print_set(suffix_cover, usize);

        prev_cover = suffix_cover;
    }
}

void del_trivial_subsets()
{
    for (int i = nsubsets - 1; i >= 1; i--)
    {
        for (int j = i - 1; j >= 0; j--)
        {
            if (is_superset(subsets[j], subsets[i], usize))
            {
                // Implicitly remove the subset by moving
                // to the end and decreasing size of list

                // Bubble to last position
                Bitset temp = subsets[i];

                for (int k = i; k < nsubsets - 1; k++)
                {
                    subsets[k] = subsets[k+1];
                }
                subsets[nsubsets - 1] = temp;

                nsubsets--;

                break;
            }
        }
    }

    // Update nsubsets_word and nsubsets_byte
    nsubsets_word = (nsubsets + int_sz - 1) / int_sz;
    nsubsets_byte = nsubsets_word * sizeof(int);
}


void read_sets(char *filename)
{
    FILE *file;
    char *empty_buf = NULL;
    char **buf_ptr = &empty_buf;
    size_t buf_sz = 0;

    // Open file
    if (!(file = fopen(filename, "r")))
    {
        printf("Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read usize and nsubsets
    if (fscanf(file, "%u", &usize));
    if (fscanf(file, "%u", &nsubsets));

    usize_word = (usize + int_sz - 1) / int_sz;
    nsubsets_word = (nsubsets + int_sz - 1) / int_sz;

    usize_byte = usize_word * sizeof(int);
    nsubsets_byte = nsubsets_word * sizeof(int);

    minimum = usize + 1;

    set_cover = alloc_set(usize);
    selection = alloc_set(nsubsets);
    solution = alloc_set(nsubsets);

    subsets = malloc(nsubsets * sizeof(Bitset));


    // Read and initialize subsets
    for (int i = 0; !feof(file) && i < nsubsets; i++)
    {
        if (getline(buf_ptr, &buf_sz, file) < 0)
        {
            if (!feof(file)) {
                perror("Error from getline");
                exit(EXIT_FAILURE);
            }
        }

        // Ignore empty line (empty set)
        if (strlen(*buf_ptr) > 1)
        {
            Bitset subset = alloc_set(usize);

            char **buf_ptr2 = buf_ptr;
            char *s;
            int elem;

            while ((s = strsep(buf_ptr2, " ")))
            {
                sscanf(s, "%d", &elem);
                add_elem(subset, elem-1); // Files are 1-indexed. We want 0-indexed
            }

            subsets[i] = subset;
        }
        else
        {
            // There may be an empty set
            if (i > 0)
            {
                subsets[i] = alloc_set(usize);
            }
            // But if it's at the first subset
            // it's probably just a reading error
            else
            {
                i--;
            }
        }


        free(*buf_ptr);
        buf_ptr = &empty_buf;
        buf_sz = 0;
    }


    // Close file
    fclose(file);
}

void free_sets()
{
    for (int i = 0; i < nsubsets; i++)
    {
        free(subsets[i]);
    }
    for (int i = 0; i < nsubsets; i++)
    {
        free(suffix_covers[i]);
    }

    free(suffix_covers);
    free(set_cover);
    free(selection);
    free(solution);
    free(subsets);
}

int compare_set_sz(const void *set_ptr1, const void *set_ptr2)
{
    Bitset set1 = *(Bitset *)set_ptr1;
    Bitset set2 = *(Bitset *)set_ptr2;

    // Only for use on comparing subsets of U
    return set_sz(set2, usize) - set_sz(set1, usize);
}

void print_subset(int i)
{
    print_set(subsets[i], usize);
}