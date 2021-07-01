#ifndef mem_h
#define mem_h
#include <stdlib.h>

void* ReallocProper(void** ptr, size_t typeSize, size_t newCount, size_t oldCount, void (*copyConstructor)(void*, void*));
void* ShrinkAlloc(void** ptr, size_t typeSize, size_t newCount, void (*copyConstructor)(void*, void*));

/** TODO: Move to another file.

#ifndef G_MemoryManager_PG_SIZE
#define G_MemoryManager_PG_SIZE 1024
#endif

typedef struct memory_manager {
    uint8_t bitset[G_MemoryManager_PG_SIZE / 8];
    node pages;
} memory_manager;

void GInitMemoryManager();
void* GAlloc(size_t size);
void* GRealloc(void* ptr, size_t newSize);
void* GCalloc(size_t count, size_t size);
void* GRecalloc(void* ptr, size_t count, size_t size);
void GDestroyMemoryManager();

void InitMemoryManager(memory_manager* manager);
void* Alloc(memory_manager* manager, size_t size);
void* Realloc(memory_manager* manager, void* ptr, size_t newSize);
void* Calloc(memory_manager* manager, size_t count, size_t size);
void* Recalloc(memory_manager* manager, void* ptr, size_t count, size_t size);
void DestroyMemoryManager(memory_manager* manager);
**/

#endif