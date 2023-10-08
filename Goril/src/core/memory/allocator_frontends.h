#pragma once
#include "defines.h"

#include "allocator_types.h"
#include "memory_debug_tools.h"


#ifdef GR_DIST

// void* Alloc(Allocator* allocator, u64 size, MemTag memtag);
#define Alloc(allocator, size, memtag) (allocator)->BackendAlloc((allocator), (size), MIN_ALIGNMENT)
// void* AlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag);
#define AlignedAlloc(allocator, size, alignment, memtag) (allocator)->BackendAlloc((allocator), (size), (alignment))
// void* Realloc(Allocator* allocator, void* block, u64 newSize);
#define Realloc(allocator, block, newSize) (allocator)->BackendRealloc((allocator), (block), (newSize))
// void Free(Allocator* allocator, void* block);
#define Free(allocator, block) (allocator)->BackendFree((allocator), (block))

#else // if not gr dist

// void* Alloc(Allocator* allocator, u64 size, MemTag memtag);
#define Alloc(allocator, size, memtag) DebugAlignedAlloc((allocator), (size), MIN_ALIGNMENT, (memtag))
// void* AlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag);
#define AlignedAlloc(allocator, size, alignment, memtag) DebugAlignedAlloc((allocator), (size), (alignment), (memtag))
// void* Realloc(Allocator* allocator, void* block, u64 newSize);
#define Realloc(allocator, block, newSize) DebugRealloc((allocator), (block), (newSize))
// void Free(Allocator* allocator, void* block);
#define Free(allocator, block) DebugFree((allocator), (block))

#endif

// ================================== Global allocators (not an allocator type) =================================================================================================================================================
// These allocators call malloc instead of getting their memory from a parent allocator, but they're just freelist allocators
bool CreateGlobalAllocator(size_t arenaSize, Allocator* out_allocator, size_t* out_stateSize, u64* out_arenaStart);
void DestroyGlobalAllocator(Allocator allocator);

// ================================== Freelist allocator =================================================================================================================================================
Allocator CreateFreelistAllocator(Allocator* parentAllocator, size_t arenaSize);
void DestroyFreelistAllocator(Allocator* parentAllocator, Allocator allocator);
size_t FreelistGetFreeNodes(void* backendState);
u32 GetFreelistAllocHeaderSize();

// ==================================== Bump allocator ================================================================================================================================================
Allocator CreateBumpAllocator(Allocator* parentAllocator, size_t arenaSize);
void DestroyBumpAllocator(Allocator* parentAllocator, Allocator allocator);


// ===================================== Pool allocator =============================================================================================================================================
// Size of blocks this allocator returns, and amount of blocks in this allocator.
// all blocks created by this allocator are aligned on allocSize (provided it is a power of two)
Allocator CreatePoolAllocator(Allocator* parentAllocator, u32 blockSize, u32 poolSize);
void DestroyPoolAllocator(Allocator* parentAllocator, Allocator allocator);