#pragma once


#ifdef GR_DIST

#define START_MEMORY_DEBUG_SUBSYS()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS()

#define PRINT_MEMORY_STATS()

#include "../asserts.h"
#define MARK_ALLOCATOR(allocator) GRASSERT_MSG(false, "Don't forget to remove allocator markers")

#define REGISTER_ALLOCATOR(arenaStart, arenaEnd, stateSize, out_allocatorId, type, parentAllocator, name, allocator)
#define UNREGISTER_ALLOCATOR(allocatorId, allocatorType)
#define DEBUG_FLUSH_ALLOCATOR(pAllocator)

#endif


/*
* All currently available memory tools:
* Keeping track of total user allocations (in bytes and allocations)
* Keeping track of user allocations per tag (in bytes and allocations)
* Printing all outstanding memory allocations, their size, tag, and the file and line they were allocated on
* Asserting when a block is freed that was never allocated 
* Asserting when a block is realloced that was never allocated
* Keeping track of allocators
* 
* TODO:
* Printing an allocator hierarchy
* Printing the amount of memory taken by allocator state
* Printing how much memory is in use in total
* Printing how much memory of each allocator is in use
* Asserting when a block is freed with the wrong allocator
* Logging whether the address of a wrong free or realloc falls within the domain of an allocator or is completely outside of the game's reserved memory
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

#define START_MEMORY_DEBUG_SUBSYS() _StartMemoryDebugSubsys()
#define SHUTDOWN_MEMORY_DEBUG_SUBSYS() _ShutdownMemoryDebugSubsys()

void _PrintMemoryStats();

#define PRINT_MEMORY_STATS() _PrintMemoryStats()

void _MarkAllocator(Allocator* allocator);

// Marks an allocator for debugging. replaces all allocator allocations with malloc, allowing external memory debuggers to be used.
// TODO: This disables seeing how much of the allocator is being used (for now)
#define MARK_ALLOCATOR(allocator) _MarkAllocator(allocator);

void _RegisterAllocator(u64 arenaStart, u64 arenaEnd, u32 stateSize, u32* out_allocatorId, AllocatorType type, Allocator* parentAllocator, const char* name, Allocator* allocator);
void _UnregisterAllocator(u32 allocatorId, AllocatorType allocatorType);
u32 _DebugFlushAllocator(Allocator* allocator);

#define REGISTER_ALLOCATOR(arenaStart, arenaEnd, stateSize, out_allocatorId, type, parentAllocator, name, allocator) _RegisterAllocator(arenaStart, arenaEnd, stateSize, out_allocatorId, type, parentAllocator, name, allocator)
#define UNREGISTER_ALLOCATOR(allocatorId, allocatorType) _UnregisterAllocator(allocatorId, allocatorType)
#define DEBUG_FLUSH_ALLOCATOR(pAllocator) _DebugFlushAllocator(pAllocator)

void* DebugAlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag, const char* file, u32 line);
void* DebugRealloc(Allocator* allocator, void* block, u64 newSize, const char* file, u32 line);
void DebugFree(Allocator* allocator, void* block, const char* file, u32 line);

#endif
