#pragma once
#include "defines.h"



// Forward declaring mem_tag enum
typedef enum mem_tag
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
	MAX_MEMORY_TAGS
} mem_tag;

// Forward declaring allocator struct
typedef struct Allocator Allocator;

typedef void* (*PFN_BackendAlloc)(Allocator* allocator, u64 size, mem_tag tag, u32 alignment);
typedef void* (*PFN_BackendReAlloc)(Allocator* allocator, void* block, u64 size);
typedef void (*PFN_BackendFree)(Allocator* allocator, void* block);


// The client of this struct should not touch it's internals
typedef struct Allocator
{
	PFN_BackendAlloc BackendAlloc;
	PFN_BackendReAlloc BackendReAlloc;
	PFN_BackendFree BackendFree;
	void* backendState;
} Allocator;


u32 GetAllocHeaderSize();

u64 GetBlockSize(void* block);

// TODO: replace this with macros that send in the file and line as well
static inline void* AlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment)
{
	return allocator->BackendAlloc(allocator, size, tag, alignment);
}

static inline void* ReAlloc(Allocator* allocator, void* block, u64 size)
{
	return allocator->BackendReAlloc(allocator, block, size);
}

static inline void Free(Allocator* allocator, void* block)
{
	allocator->BackendFree(allocator, block);
}

static inline void* Alloc(Allocator* allocator, u64 size, mem_tag tag)
{
	return AlignedAlloc(allocator, size, tag, MIN_ALIGNMENT);
}