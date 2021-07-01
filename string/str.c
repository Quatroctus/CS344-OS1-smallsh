#include "str.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "../memory/mem.h"

/**
 * Constructs a string struct from a char*.
 * Mallocs a string if the string location is NULL.
 * @param s Is the location to store the string.
 * @param str Is the initial contents of the string struct.
 * @return The constructed string.
**/
string* ConstructStr(string* s, const char* str) {
    if (s == NULL) s = malloc(sizeof(string));
    s->heap = false;
    s->size = 32;
    s->str = (char*) s->s;
    if (str != NULL) SetCStr(s, str);
    return s;
}

/**
 * Properly copies the second string into the first string.
 * @param v1 The pointer to the destination string.
 * @param v2 The pointer to the source string.
**/
void CopyConstructStr(void* v1, void* v2) {
    string* s1 = v1;
    string* s2 = v2;
    *s1 = *s2;
    if (!s1->heap) s1->str = s1->s;
//    printf("Copy Constructor: %s|%s|\n", s1->str, s2->str);
}

/**
 * Perform a deep copy on the source string to the destination string.
 * Mallocs a string if dest is NULL.
 * @param dest Is the destination string struct.
 * @param src Is the string struct to copy from.
 * @return The dest string.
**/
string* DeepCopy(string* dest, string* src) {
    if (dest == NULL) dest = malloc(sizeof(string));
    *dest = *src;
    if (src->heap) {
        dest->str = malloc(sizeof(char) * src->size);
        strcpy(dest->str, src->str);
    } else {
        dest->str = dest->s;
    }
    return dest;
}

/**
 * Appends a char* to the end of the current string.
 * @param dest The string struct to append to.
 * @param str The char* to append to the string struct.
 * @return The dest string.
**/
string* AppendCStr(string* dest, const char* src) {
    size_t newLength = dest->length + strlen(src) + 1;
    if (newLength > dest->size) {
        if (dest->heap) ReallocProper((void**) &dest->str, sizeof(char), newLength, dest->size, NULL);
        else {
            dest->str = malloc(sizeof(char) * newLength);
            strcpy(dest->str, dest->s);
            dest->heap = true;
        }
        dest->size = newLength;
    }
    strcpy(dest->str + (sizeof(char) * dest->length), src);
    dest->length = newLength - 1;
    return dest;
}

/**
 * Appends a string struct to another string struct.
 * @param dest Is the destination string.
 * @param src Is the string to append to the dest.
 * @return The dest string.
**/
string* AppendString(string* dest, string* src) {
    size_t newLength = dest->length + src->length + 1;
    if (newLength > dest->size) {
        if (dest->heap) {
            ReallocProper((void**) &dest->str, sizeof(char), newLength, dest->size, NULL);
        } else {
            dest->str = malloc(sizeof(char) * newLength);
            strcpy(dest->str, dest->s);
            dest->heap = true;
        }
        dest->size = newLength;
    }
    memcpy(dest->str + (sizeof(char) * dest->length), src->str, src->length + 1);
    //strcpy(dest->str + (sizeof(char) * dest->length), src->str);
    dest->length = newLength - 1;
    return dest;
}

/**
 * Sets the contents of a string struct from a char*.
 * @param dest Is the string to set the contents of.
 * @param src Is the char* data to set the contents to.
 * @return The dest string.
**/
string* SetCStr(string* dest, const char* src) {
    size_t length = strlen(src) + 1;
    if (dest->size < length) {
        if (dest->heap) ReallocProper((void**) &dest->str, sizeof(char), length, dest->size, NULL);
        else {
            dest->str = malloc(sizeof(char) * length);
            dest->heap = true;
        }
        dest->size = length + 1;
    }
    strcpy(dest->str, src);
    dest->length = length - 1;
//    dest->size = (length+1) <= 32 ? 32 : length+1;
    return dest;
}

/**
 * Sets the contents of a string struct from another string struct. (Deep Copy)
 * @param dest Is the string to set the contents of.
 * @param src Is the string to copy the contents from.
 * @return The dest string.
**/
string* SetString(string* dest, string* src) {
    if (dest->size < src->length) {
        if (dest->heap) ReallocProper((void**) &dest->str, sizeof(char), src->length, dest->size, NULL);
        else {
            dest->str = malloc(sizeof(char) * src->length);
            dest->heap = true;
        }
        dest->size = src->length;
    }
    strcpy(dest->str, src->str);
    dest->length = src->length;
    return dest;
}

/**
 * Stores a substring of src in dest from start to end.
 * @param dest The destination string struct.
 * @param src The source string struct.
 * @param start The starting index(inclusive).
 * @param end The ending index(inclusive).
 * @return The dest string if the substring was successfully made.
**/
string* SubString(string* dest, string* src, size_t start, size_t end) {
    if (start > end) return NULL;
    if (dest == NULL || !dest->heap) ConstructStr(dest, NULL);
    end = end <= src->length ? end : src->length;
    size_t length = end - start + 1;
    if (dest->size < length) {
        if (dest->heap) {
            void* temp = ReallocProper((void**) &dest->str, sizeof(char), length, dest->size, NULL);
            if (temp == NULL) return NULL;
        } else {
            dest->str = malloc(sizeof(char) * length);
            dest->heap = true;
        }
        dest->size = length;
    }
    memcpy((void*) dest->str, (void*) (src->str + (sizeof(char) * start)), end - start);
    dest->str[end - start + 1] = 0;
    dest->length = length - 1;
    return dest;
}

/**
 * Stores a substring of src in dest from start to end
 * and reduces dest's size to the new length.
 * @param dest The destination string struct.
 * @param src The source string struct.
 * @param start The starting index(inclusive).
 * @param end The ending index(inclusive).
 * @return The dest string if the substring was successfully made.
**/
string* SubStringReduce(string* dest, string* src, size_t start, size_t end) {
    if (dest == NULL || !dest->heap) return SubString(dest, src, start, end);
    end = end <= src->length ? end : src->length;
    size_t length = end - start + 1;
    if (length <= 32) {
        memcpy((void*) dest->s, (void*) (src->str + (sizeof(char) * start)), end - start);
        free(dest->str);
        dest->str = dest->s;
        dest->heap = false;
    } else {
        memcpy((void*) dest->str, (void*) (src->str + (sizeof(char) * start)), end - start);
        dest->str = realloc((void*) dest->str, sizeof(char) * length);
    }
    dest->str[end - start] = 0;
    dest->length = length - 1;
    dest->size = length;
    return dest;
}

/**
 * Reduces the size of a string to the length of the string.
 * @param str Is the string to reduce the size of.
 * @return The reduced string.
**/
string* ReduceString(string* str) {
    if (!str->heap || str->length + 1 == str->size) return str;
    if (str->length + 1 <= 32) {
        strcpy(str->s, str->str);
        free(str->str);
        str->str = str->s;
        str->heap = false;
        str->size = 32;
    } else {
        void* temp = realloc(str->str, str->length + 1);
        str->size = str->length + 1;
    }
    return str;
}

/**
 * Get a char at a given index. If the index is out of range returns -1.
 * @param src The string to get the char from.
 * @param index The index of the char you want.
 * @return The char at index in src or -1 if out of bounds.
**/
int GetStrChar(string* src, size_t index) {
    if (index > src->length) return -1;
    return src->str[index];
}

/**
 * Cleans up any heap allocation this string has.
 * @param string The string to destroy.
**/
void DestroyStr(string* string) {
    if (string->heap) {
        free(string->str);
    }
}
