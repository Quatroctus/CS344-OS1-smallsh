#include "mem.h"

#include <stdio.h>
#include <memory.h>

/**
 * Uses realloc to attempt to reallocate the memory and if that fails
 * it instead searches for a new block of memory with malloc.
 * If malloc fails it maintains the old block of memory and returns NULL.
 * @param ptr Is a pointer which points to the start of the old memory block.
 * @param typeSize Is the block size for this pointer.
 * @param newCount Is the new count of the blocks of memory length of typeSize.
 * @param oldCount Is the current/old count of the typeSize blocks.
 * @param copyConstructor Is an optional function to block a block of memory to the new block.
 * @return The new block of memory or NULL if the block could not be reallocated.
**/
void* ReallocProper(void** ptr, size_t typeSize, size_t newCount, size_t oldCount, void (*copyConstructor)(void*, void*)) {
    size_t newSize = typeSize * newCount, oldSize = typeSize * oldCount;
    void* temp = realloc(*ptr, newSize);
    if (temp == NULL) return NULL;
    if (temp != *ptr && copyConstructor) {
        for (size_t offset = 0; offset < oldSize; offset += typeSize) {
            copyConstructor(temp + offset, temp + offset);
        }
    }
    *ptr = temp;
    return temp;
}

/**
 * Attempts to shrink the memory block using realloc, if realloc fails malloc is used.
 * If malloc fails it maintains the old block of memory and returns NULL.
 * @param ptr Is a pointer which points to the start of the old memory block.
 * @param typeSize Is the block size for this pointer.
 * @param newCount Is the count of how many typeSize blocks there are.
 * @param copyConstructor Is an optional function to block a block of memory to the new block.
 * @return The new block of memory or NULL if the block could not be reallocated.
**/
void* ShrinkAlloc(void** ptr, size_t typeSize, size_t newCount, void (*copyConstructor)(void*, void*)) {
    void* temp = *ptr;
    size_t newSize = typeSize * newCount;
    if ((*ptr = realloc(*ptr, newSize)) == NULL) {
        *ptr = malloc(newSize);
        if (*ptr == NULL) {
            *ptr = temp;
            return NULL;
        }
        if (copyConstructor) {
            for (size_t offset = 0; offset < newSize; offset += typeSize) {
                copyConstructor(*ptr + offset, temp + offset);
            }
        } else {
            memcpy(*ptr, temp, newSize);
        }
        free(temp);
    }
    return *ptr;
}
