#pragma once

#ifndef GR_DIST

#include "allocator_types.h"


void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment);
void* DebugRealloc(Allocator* allocator, void* block, u64 newSize);
void DebugFree(Allocator* allocator, void* block);


#endif
