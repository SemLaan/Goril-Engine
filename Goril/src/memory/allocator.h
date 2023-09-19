#pragma once
#include "defines.h"



// Forward declaring mem_tag enum
typedef enum mem_tag mem_tag;

typedef void* (*PFN_BackendAlloc)(void* backendState, u64 size);
typedef bool (*PFN_BackendTryReAlloc)(void* backendState, void* block, u64 oldSize, u64 newSize);
typedef void (*PFN_BackendFree)(void* backendState, void* block, u64 size);


// The client of this struct should not touch it's internals
typedef struct Allocator
{
	PFN_BackendAlloc BackendAlloc;
	PFN_BackendTryReAlloc BackendTryReAlloc;
	PFN_BackendFree BackendFree;
	void* backendState;
} Allocator;


GRAPI u32 GetAllocHeaderSize();

GRAPI u64 GetBlockSize(void* block);

GRAPI void* AlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment);
GRAPI void* ReAlloc(Allocator* allocator, void* block, u64 size);
GRAPI void Free(Allocator* allocator, void* block);

GRAPI inline void* Alloc(Allocator* allocator, u64 size, mem_tag tag)
{
	return AlignedAlloc(allocator, size, tag, MIN_ALIGNMENT);
}