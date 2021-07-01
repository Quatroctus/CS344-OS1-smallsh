#include <stdlib.h>
#include <stdbool.h>

/**=================================================================|
 * A basic string struct.                                           |
 * =================================================================|
 * >>> Special Information.                                         |
 * If the string is 32 characters including the null terminator,    |
 * the string will be stored in s. Which if the string is located   |
 * on the stack this means there will be no heap allocation for     |
 * this string. If the string is located on the heap that means     |
 * if it is within the 32 character limit the string is stored in   |
 * contiguous memory and will not be fragmented.                    |
 * =================================================================|
 * >>> Member Information.                                          |
 * bool heap If the underlying C string is stored on the heap.      |
 * size_t length The number of characters excluding the null-       |
 *      terminator.                                                 |
 * size_t size The size of the C string memory block.               |
 * char* str The accurate pointer to the string memory block.       |
 * char s[32] The 32 chars stored inside the string.                |
 * =================================================================|
 * Struct Size: 64 bytes on 64 bit systems and 48 on 32 bit systems.|
 * =================================================================|
**/
typedef struct string {
    bool heap;
    size_t length, size;
    char* str;
    char s[32];
} string;

string* ConstructStr(string* s, const char* str);
void CopyConstructStr(void* s1, void* s2);
string* DeepCopy(string* dest, string* src);
string* AppendCStr(string* dest, const char* src);
string* AppendString(string* dest, string* src);
string* SetCStr(string* dest, const char* str);
string* SetString(string* dest, string* src);
string* SubString(string* dest, string* src, size_t start, size_t end);
string* SubStringReduce(string* dest, string* src, size_t start, size_t end);
string* ReduceString(string* str);
int GetStrChar(string* src, size_t index);
void DestroyStr(string* string);
