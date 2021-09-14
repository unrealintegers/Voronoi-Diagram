/*
 *  Utility functions for safe memory allocation
 *  and a python-inspired implementation dynamic arrays (lists)
 */

#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>

#include"utils.h"

#define INIT_SIZE 12
#define GROWTH_FACTOR 1.5f

void * safeMalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        printf("malloc failed, exiting...\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void * safeRealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    if (ptr == NULL) {
        printf("realloc failed, exiting...\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

FILE * safeOpen(const char *path, const char *mode) {
    FILE *f = fopen(path, mode);
    if (f == NULL) {
        printf("file %s not found, aborting...\n", path);
        exit(EXIT_FAILURE);
    }
    return f;
}

list_t * initList(void) {
    list_t *list = (list_t *) safeMalloc(sizeof(list_t));

    *list = (list_t) {.index = 0,
                      .curSize = 0,
                      .maxSize = INIT_SIZE,
                      .arr = safeMalloc(INIT_SIZE * sizeof(void *)),
                      ._next = NULL,
                      .freeElem = free};

    return list;
}

void appendList(list_t *list, void *elem) {
    if (list->curSize == list->maxSize) {
        list->maxSize = (long) (list->maxSize * GROWTH_FACTOR);

        list->arr = safeRealloc(list->arr, list->maxSize * sizeof(void *));
    }

    list->arr[list->curSize] = elem;
    list->curSize++;
}

void * getList(list_t *list, long index) {
    if (index >= list->curSize) {
        printf("list index [%ld] out of range (%ld), exiting...\n", 
               index, list->curSize);
        exit(EXIT_FAILURE);
    }
    return list->arr[index];
}

void iterList(list_t *list, void **next) {
    list->index = 0;
    list->_next = next;
}

bool nextList(list_t *list) {
    if (list->index >= list->curSize) {
        return false;
    }
    
    if (list->_next == NULL) {
        return false;
    }

    *(list->_next) = list->arr[list->index++];
    return true;
}

void freeList(list_t *list) {
    for (int i = 0; i < list->curSize; i++) {
        if (list->freeElem != NULL) list->freeElem(list->arr[i]);
    }

    free(list->arr);
    free(list);
}

void iiSortList(list_t *list) {
    // index represents the index of the currently inserting element
    for (int index = 0; index < list->curSize; index++) {
        void *end = list->arr[index];
        
        int i;
        for (i = index; i > 0; i--) {
            if (!list->cmp(list->arr[i-1], end)) break;
            list->arr[i] = list->arr[i-1];
        }
        list->arr[i] = end;
    }
}
