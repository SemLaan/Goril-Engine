#include "memory/gr_memory.h"
#include "memory/mem_utils.h"


#ifdef GR_DIST
#define NO_MEMORY_DEBUGGING
#else
#define MEMORY_DEBUGGING
#endif



#ifdef MEMORY_DEBUGGING

// void* Alloc(Allocator* allocator, u64 size, mem_tag tag)
#define Alloc(allocator, size, tag) allocator->BackendAlloc(allocator, size, MIN_ALIGNMENT)
// void* AlignedAlloc(Allocator* allocator, u64 size, u32 alignment, mem_tag tag)
#define AlignedAlloc(allocator, size, alignment, tag) allocator->BackendAlloc(allocator, size, alignment)
// void* ReAlloc(Allocator* allocator, void* block, u64 size)
#define ReAlloc(allocator, block, size) allocator->BackendReAlloc(allocator, block, size)
// void Free(Allocator* allocator, void* block)
#define Free(allocator, block) allocator->BackendFree(allocator, block);

#else // if not memory debugging

// void* Alloc(Allocator* allocator, u64 size, mem_tag tag)
#define Alloc(allocator, size, tag) allocator->BackendAlloc(allocator, size, MIN_ALIGNMENT)
// void* AlignedAlloc(Allocator* allocator, u64 size, u32 alignment, mem_tag tag)
#define AlignedAlloc(allocator, size, alignment, tag) allocator->BackendAlloc(allocator, size, alignment)
// void* ReAlloc(Allocator* allocator, void* block, u64 size)
#define ReAlloc(allocator, block, size) allocator->BackendReAlloc(allocator, block, size)
// void Free(Allocator* allocator, void* block)
#define Free(allocator, block) allocator->BackendFree(allocator, block);

#endif
