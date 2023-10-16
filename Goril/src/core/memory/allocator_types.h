#pragma once

#include "defines.h"


typedef enum MemTag
{
	MEM_TAG_ALLOCATOR_STATE,
	MEM_TAG_SUB_ARENA,
	MEM_TAG_MEMORY_SUBSYS,
	MEM_TAG_LOGGING_SUBSYS,
	MEM_TAG_PLATFORM_SUBSYS,
	MEM_TAG_EVENT_SUBSYS,
	MEM_TAG_RENDERER_SUBSYS,
	MEM_TAG_INPUT_SUBSYS,
	MEM_TAG_GAME,
	MEM_TAG_TEST,
	MEM_TAG_DARRAY,
	MEM_TAG_VERTEX_BUFFER,
	MEM_TAG_INDEX_BUFFER,
	MEM_TAG_TEXTURE,
	MEM_TAG_HASHMAP,
    MEM_TAG_MEMORY_DEBUG,
	MAX_MEMORY_TAGS
} MemTag;

// Forward declaring allocator struct
typedef struct Allocator Allocator;

typedef void* (*PFN_BackendAlloc)(Allocator* allocator, u64 size, u32 alignment);
typedef void* (*PFN_BackendRealloc)(Allocator* allocator, void* block, u64 newSize);
typedef void (*PFN_BackendFree)(Allocator* allocator, void* block);


typedef enum AllocatorType
{
	ALLOCATOR_TYPE_GLOBAL,
	ALLOCATOR_TYPE_FREELIST,
	ALLOCATOR_TYPE_BUMP,
	ALLOCATOR_TYPE_POOL,
	ALLOCATOR_TYPE_MAX_VALUE,
} AllocatorType;


// The client of this struct should not touch it's internals
// **IMPORTANT* Don't copy the allocator only pointers to it
typedef struct Allocator
{
	PFN_BackendAlloc BackendAlloc;
	PFN_BackendRealloc BackendRealloc;
	PFN_BackendFree BackendFree;
	void* backendState;
    Allocator* parentAllocator;
	// Used by memory debug tools
	u32 id; // Is only usefull in debug mode, is always zero in dist mode
} Allocator;