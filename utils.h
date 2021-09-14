/*
 *  Utility functions for safe memory allocation
 *  and a python-inspired implementation dynamic arrays (lists)
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#define max(A, B) (((A) > (B)) ? (A) : (B))
#define min(A, B) (((A) > (B)) ? (B) : (A))

typedef struct DynamicArray list_t;

// "generic" dynamic array using void pointers
// supporting iteration
struct DynamicArray {
    long curSize, maxSize;
    void **arr;

    // list iteration
    long index;
    void **_next;

    // function pointer for how to free
    void (*freeElem)(void *);

    // comparison function, true if first >= second
    bool (*cmp)(void *, void *);
};

void * safeMalloc(size_t);
void * safeRealloc(void *, size_t);
FILE * safeOpen(const char *, const char *);

list_t * initList(void);
void appendList(list_t *, void *);
void * getList(list_t *, long);
void iterList(list_t *, void **);
bool nextList(list_t *);
void freeList(list_t *);

// Sorts a List using In-place Insertion Sort
void iiSortList(list_t *);

#endif
