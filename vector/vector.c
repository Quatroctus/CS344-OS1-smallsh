#include "vector.h"

#include "../memory/mem.h"

#include <stdint.h>
#include <memory.h>

/**
 * Creates a vector struct with a size of 4.
 * @param typeSize Is the size of the type the vector contains.
 * @param copyConstructor Is an optional function for performing
 *   a copy on the underlying object.
 * @param destructor Is an optional function for performing cleanup
 *   on the underlying object.
 * @return The constructed vector.
**/
vector ConstructVector(size_t typeSize, void (*copyConstructor)(void*, void*), void (*destructor)(void*)) {
    vector vec = {0, 4, typeSize, malloc(typeSize * 4), copyConstructor, destructor};
    return vec;
}

/**
 * Creates a deep copy of a vector with mallocing and memcpy.
 * @param vector Is the vector to copy.
 * @return The new vector.
**/
vector DeepCopyVector(vector* v) {
    vector vec = *v;
    vec.items = malloc(vec.size * vec.typeSize);
    if (vec.copyConstructor) {
        for (int i = 0; i < vec.length; i++) {
            vec.copyConstructor(((uint8_t*) vec.items) + vec.typeSize * i, ((uint8_t*) v->items) + v->typeSize * i);
        }
    } else {
        memcpy(vec.items, v->items, vec.typeSize * vec.size);
    }
    return vec;
}

/**
 * Creates a sub-vector of a vector.
 * @param vector The vector to create a sub-vector of.
 * @param start The starting index of the sub-vector.
 * @param end The ending index of the sub-vector.
 * @return The sub-vector.
**/
vector SubVector(vector* v, size_t start, size_t end) {
    if (start >= v->length) return ConstructVector(v->typeSize, v->copyConstructor, v->destructor);
    size_t subLength = v->length - start;
    vector vec = {0, 0, v->typeSize, NULL, v->copyConstructor};
    vec.size = end - start + 1;
    vec.length = vec.size < subLength ? vec.size : subLength;
    vec.items = malloc(vec.typeSize * vec.size);
    if (vec.copyConstructor) {
        for (int i = 0; i < vec.length; i++) {
            vec.copyConstructor(((uint8_t*) vec.items) + vec.typeSize * i, ((uint8_t*) vec.items) + vec.typeSize * (i + start));
        }
    } else {
        memcpy(vec.items, ((uint8_t*) v->items) + (v->typeSize * start), vec.length);
    }
    return vec;
}

/**
 * Appends a value to the end of the vector.
 * @param vector The vector to append to.
 * @param valuePtr The value to append to the vector.
 * @return If the value was appended to the vector.
**/
bool PushBackVector(vector* vector, void* valuePtr) {
    if (vector->length >= vector->size) {
        void* temp = ReallocProper(&vector->items, vector->typeSize, vector->size * 2, vector->size, vector->copyConstructor);
        if (temp == NULL) return false;
        vector->size *= 2;
    }
    if (vector->copyConstructor) {
        vector->copyConstructor(((uint8_t*)vector->items) + (vector->length * vector->typeSize), valuePtr);
    } else {
        memcpy(((uint8_t*)vector->items) + vector->length * vector->typeSize, valuePtr, vector->typeSize);
    }
    vector->length++;
    return true;
}

/**
 * Replaces a value in the vector.
 * Sets vector::items at index to valuePtr.
 * @param vector The vector to insert a value into.
 * @param valuePtr The value to insert into the vector.
 * @param index The index into the vector to insert at.
**/
void InsertVector(vector* vector, void* valuePtr, size_t index) {
    if (index >= vector->length) return;
    if (vector->destructor)
        vector->destructor(((uint8_t*) vector->items) + vector->typeSize * index);
    if (vector->copyConstructor) {
        vector->copyConstructor(((uint8_t*)vector->items) + index * vector->typeSize, valuePtr);
    } else {
        memcpy(((uint8_t*)vector->items) + index * vector->typeSize, valuePtr, vector->typeSize);
    }
}

/**
 * Places a value in the vector.
 * Shifts [index:vector::length] right one and sets index to valuePtr.
 * @param vector The vector to insert a value into.
 * @param valuePtr The value to place into the vector.
 * @param index The index into the vector to place at.
 * @return If the value was successfully placed into the vector.
**/
bool InjectVector(vector* vector, void* valuePtr, size_t index) {
    if (index >= vector->length) return false;
    if (vector->length >= vector->size) {
        void* temp = ReallocProper(&vector->items, vector->typeSize, vector->size * 2, vector->size, vector->copyConstructor);
        if (temp == NULL) return false;
        vector->size *= 2;
    }
    if (vector->copyConstructor) {
        for (int i = index; i < vector->length-1; i++) {
            vector->copyConstructor(
                ((uint8_t*) vector->items) + (i + 1) * vector->typeSize,
                ((uint8_t*) vector->items) + i * vector->typeSize
            );
        }
        vector->copyConstructor(((uint8_t*) vector->items) + index * vector->typeSize, valuePtr);
    } else {
        memcpy(((uint8_t*) vector->items) + (index + 1) * vector->typeSize, ((uint8_t*) vector->items) + index * vector->typeSize, (vector->length - index) * vector->typeSize);
        memcpy(((uint8_t*)vector->items) + index * vector->typeSize, ((uint8_t*) valuePtr), vector->typeSize);
    }
    vector->length++;
    return true;
}

/**
 * Remove a value from the vector at index.
 * @param vector The vector to remove a value from.
 * @param index The index to remove from the vector.
**/
void RemoveVector(vector* vector, size_t index) {
    if (index >= vector->length) return;
    if (vector->destructor)
        vector->destructor(((uint8_t*) vector->items) + vector->typeSize * index);
    if (vector->copyConstructor) {
        for (int i = index; i < vector->length-1; i++) {
            vector->copyConstructor(
                ((uint8_t*) vector->items) + index * vector->typeSize,
                ((uint8_t*) vector->items) + (index + 1) * vector->typeSize
            );
        }
    } else {
        memcpy(((uint8_t*) vector->items) + index * vector->typeSize, ((uint8_t*) vector->items) + (index + 1) * vector->typeSize, (vector->length - index - 1) * vector->typeSize);
    }
    vector->length--;
}

/**
 * Shrink a vectors size to the length.
 * @param vector The vector to shrink the size of.
 * @return If the vector was successfully shrunk to vector::length.
**/
bool ShrinkVector(vector* vector) {
    if (vector->size - vector->length) {
        void* temp = ShrinkAlloc(&vector->items, vector->typeSize, vector->length, vector->copyConstructor);
        if (temp == NULL) return false;
        vector->size = vector->length;
    }
    return true;
}

/**
 * Reduces the length and size of a vector to a specific length.
 * @param vector The vector to reduce the size of.
 * @param length The length to reduce the vector to.
 * @return If the vector was successfully reduced.
**/
bool ReduceVector(vector* vector, size_t length) {
    if (vector->size - length > 0) {
        if (vector->destructor) {
            for (int i = length; i < vector->length; i++) {
                vector->destructor(((uint8_t*) vector->items) + vector->typeSize * i);
            }
        }
        void* temp = ShrinkAlloc(&vector->items, vector->typeSize, length, vector->copyConstructor);
        if (temp == NULL) return false;
        vector->size = length;
        return true;
    }
    return false;
}

/**
 * Destroys all items stored and sets length to 0.
 * @param vector The vector to clear.
**/
void ClearVector(vector* vector) {
    if (vector->destructor) {
        size_t size = vector->typeSize * vector->length;
        for (size_t offset = 0; offset < size; offset += vector->typeSize) {
            vector->destructor(((uint8_t*) vector->items) + offset);
        }
    }
    vector->length = 0;
}

/**
 * Cleans up vector heap allocation.
 * @param vector The vector to destroy.
**/
void DestroyVector(vector* vector) {
    if (vector->destructor) {
        size_t size = vector->typeSize * vector->length;
        for (size_t offset = 0; offset < size; offset += vector->typeSize) {
            vector->destructor(((uint8_t*) vector->items) + offset);
        }
    }
    free(vector->items);
}
