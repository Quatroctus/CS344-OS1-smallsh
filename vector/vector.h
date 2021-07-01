#ifndef vector_h
#define vector_h
#include <stdlib.h>
#include <stdbool.h>

/**=================================================================|
 * A contiguous dynamic array.                                      |
 * =================================================================|
 * >>> Special Information.                                         |
 * =================================================================|
 * Member Information.                                              |
 * size_t length The number of items currently stored.              |
 * size_t size The max number of items that can currently be stored.|
 * size_t typeSize The size of each item.                           |
 * void* items The pointer to the start of the memory block.        |
 * void (*copyConstructor)(void*, void*) An optional parameter      |
 *      it's purpose is to apply custom copy behavior when the data |
 *      is moved from location to location. When NULL memcpy is     |
 *      used to move the data instead.                              |
 * void (*destructor)(void*) An optional parameter it's purpose is  |
 *      to clean up or destroy the underlying object.               |
 * =================================================================|
 * Struct Size: 48 bytes on 64 bit systems and 24 on 32 bit systems.|
 * =================================================================|
**/
typedef struct vector {
    size_t length, size, typeSize;
    void* items;
    void (*copyConstructor)(void*, void*);
    void (*destructor)(void*);
} vector;

vector ConstructVector(size_t typeSize, void (*copyConstruct)(void*, void*), void (*destructor)(void*));
vector DeepCopyVector(vector* vector);
vector SubVector(vector* vector, size_t start, size_t end);
bool PushBackVector(vector* vector, void* valuePtr);
void InsertVector(vector* vector, void* valuePtr, size_t index);
bool InjectVector(vector* vector, void* valuePtr, size_t index);
void RemoveVector(vector* vector, size_t index);
bool ShrinkVector(vector* vector);
bool ReduceVector(vector* vector, size_t length);
void ClearVector(vector* vector);
void DestroyVector(vector* vector);
#endif
