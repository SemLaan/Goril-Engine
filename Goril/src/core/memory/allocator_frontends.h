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
#define Alloc(allocator, size, memtag) DebugAlignedAlloc((allocator), (size), MIN_ALIGNMENT, (memtag), __FILE__, __LINE__)
// void* AlignedAlloc(Allocator* allocator, u64 size, u32 alignment, MemTag memtag);
#define AlignedAlloc(allocator, size, alignment, memtag) DebugAlignedAlloc((allocator), (size), (alignment), (memtag), __FILE__, __LINE__)
// void* Realloc(Allocator* allocator, void* block, u64 newSize);
#define Realloc(allocator, block, newSize) DebugRealloc((allocator), (block), (newSize), __FILE__, __LINE__)
// void Free(Allocator* allocator, void* block);
#define Free(allocator, block) DebugFree((allocator), (block), __FILE__, __LINE__)

#endif

// ================================== Global allocators (not an allocator type) =================================================================================================================================================
// These allocators call malloc instead of getting their memory from a parent allocator, but they're just freelist allocators
bool CreateGlobalAllocator(const char* name, size_t arenaSize, Allocator** out_allocator, size_t* out_stateSize, u64* out_arenaStart);
void DestroyGlobalAllocator(Allocator* allocator);

// ================================== Freelist allocator =================================================================================================================================================
void CreateFreelistAllocator(const char* name, Allocator* parentAllocator, size_t arenaSize, Allocator** out_allocator);
void DestroyFreelistAllocator(Allocator* allocator);
size_t FreelistGetFreeNodes(void* backendState);
u32 GetFreelistAllocHeaderSize();
u64 GetFreelistAllocatorArenaUsage(Allocator* allocator);

// ==================================== Bump allocator ================================================================================================================================================
void CreateBumpAllocator(const char* name, Allocator* parentAllocator, size_t arenaSize, Allocator** out_allocator);
void DestroyBumpAllocator(Allocator* allocator);
u64 GetBumpAllocatorArenaUsage(Allocator* allocator);

// ===================================== Pool allocator =============================================================================================================================================
// Size of blocks this allocator returns, and amount of blocks in this allocator.
// all blocks created by this allocator are aligned on allocSize (provided it is a power of two)
void CreatePoolAllocator(const char* name, Allocator* parentAllocator, u32 blockSize, u32 poolSize, Allocator** out_allocator);
void DestroyPoolAllocator(Allocator* allocator);
void FlushPoolAllocator(Allocator* allocator);
u64 GetPoolAllocatorArenaUsage(Allocator* allocator);
