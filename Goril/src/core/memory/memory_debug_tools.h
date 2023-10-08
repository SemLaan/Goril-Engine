#pragma once


#ifdef GR_DIST

#define START_MEMORY_DEBUG_SUBSYS()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS()

#define PRINT_MEMORY_STATS()

#endif


/*
* All currently available memory tools:
* Keeping track of total user allocations (in bytes and allocations)
* Keeping track of user allocations per tag (in bytes and allocations)
* Printing all outstanding memory allocations, their size, tag, and the file and line they were allocated on
* Asserting when a block is freed that was never allocated
* Asserting when a block is realloced that was never allocated
* 
* TODO:
* Asserting when a block is freed with the wrong allocator
* Flaging an allocator for additional testing, in two ways:
* 1 Freeing becomes a no-op, instead the state of the memory is captured and at some later point compared, in order to find use after free bugs
* 2 Allocations get extra space on either side filled with zeroes, when the allocation is freed, 
    it is checked whether everything is still zeroes, to see if an allocation is exceeding it's boundaries
* 
*/


#ifndef GR_DIST

#include "allocator_types.h"

void _StartMemoryDebugSubsys();
void _ShutdownMemoryDebugSubsys();

void _PrintMemoryStats();

#define START_MEMORY_DEBUG_SUBSYS() _StartMemoryDebugSubsys()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS() _ShutdownMemoryDebugSubsys()

#define PRINT_MEMORY_STATS() _PrintMemoryStats()

void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag, const char* file, u32 line);
void* DebugRealloc(Allocator* allocator, void* block, u64 newSize, const char* file, u32 line);
void DebugFree(Allocator* allocator, void* block, const char* file, u32 line);


#endif
