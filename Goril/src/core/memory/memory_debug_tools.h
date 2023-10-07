#pragma once


#ifdef GR_DIST

#define START_MEMORY_DEBUG_SUBSYS()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS()


#endif



#ifndef GR_DIST

#include "allocator_types.h"

void StartMemoryDebugSubsys();
void ShutdownMemoryDebugSubsys();

#define START_MEMORY_DEBUG_SUBSYS() StartMemoryDebugSubsys()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS() ShutdownMemoryDebugSubsys()

void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment);
void* DebugRealloc(Allocator* allocator, void* block, u64 newSize);
void DebugFree(Allocator* allocator, void* block);


#endif
