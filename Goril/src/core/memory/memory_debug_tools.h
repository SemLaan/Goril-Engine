#pragma once


#ifdef GR_DIST

#define START_MEMORY_DEBUG_SUBSYS()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS()

#define PRINT_MEMORY_STATS()

#endif



#ifndef GR_DIST

#include "allocator_types.h"

void _StartMemoryDebugSubsys();
void _ShutdownMemoryDebugSubsys();

void _PrintMemoryStats();

#define START_MEMORY_DEBUG_SUBSYS() _StartMemoryDebugSubsys()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS() _ShutdownMemoryDebugSubsys()

#define PRINT_MEMORY_STATS() _PrintMemoryStats()

void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag);
void* DebugRealloc(Allocator* allocator, void* block, u64 newSize);
void DebugFree(Allocator* allocator, void* block);


#endif
