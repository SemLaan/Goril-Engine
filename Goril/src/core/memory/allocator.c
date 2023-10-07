#include "allocator.h"
#include <core/asserts.h>
#include <core/memory/allocator_backends.h>

// TODO: remove this because it is now in allocator backends
typedef struct AllocHeader
{
    void* start;
    u32 size; // Size of the client allocation
    u32 alignment;
#ifndef GR_DIST
    mem_tag tag; // Only for debugging, gets optimized out of dist builds
#endif
} AllocHeader;

// TODO: remove this because it should now be in allocator backends
u32 GetAllocHeaderSize()
{
    return sizeof(AllocHeader);
}

// TODO: remove this and make it specific to allocators, because allocators might store size in different ways
u64 GetBlockSize(void* block)
{
    AllocHeader* header = (AllocHeader*)block - 1;
    return header->size;
}
