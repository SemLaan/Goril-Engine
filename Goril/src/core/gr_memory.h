#pragma once
#include "defines.h"
#include "memory/allocator.h"
#include "memory/allocator_backends.h"

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

typedef struct GlobalAllocators
{
	Allocator temporary;
} GlobalAllocators;

extern GlobalAllocators* g_Allocators;

bool InitializeMemory(size_t requiredMemory, size_t subsysMemoryRequirement);

void ShutdownMemory();

GRAPI inline Allocator* GetGlobalAllocator();

#ifndef GR_DIST
GRAPI inline void AllocInfo(size_t size, mem_tag tag);

GRAPI inline void ReAllocInfo(i64 sizeChange);

GRAPI inline void FreeInfo(size_t size, mem_tag tag);
#endif // !GR_DIST

GRAPI inline void Zero(void* block, size_t size);

// Copies memory from source to destination, also works if source and destination overlap
GRAPI inline void MemCopy(void* destination, void* source, size_t size);

GRAPI inline const size_t& GetMemoryUsage();

GRAPI inline const u64& GetNetAllocations();

GRAPI void PrintMemoryStats();
