#include "memory_debug_tools.h"

#ifndef GR_DIST


void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment)
{
    return allocator->BackendAlloc(allocator, size, alignment);
}

void* DebugRealloc(Allocator* allocator, void* block, u64 newSize)
{
    return allocator->BackendRealloc(allocator, block, newSize);
}

void DebugFree(Allocator* allocator, void* block)
{
    allocator->BackendFree(allocator, block);
}


#endif
