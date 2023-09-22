#pragma once
#include "defines.h"
#include "memory/allocator.h"
#include "memory/allocator_backends.h"


typedef struct GlobalAllocators
{
	Allocator temporary;
} GlobalAllocators;

extern GlobalAllocators* g_Allocators;

bool InitializeMemory(size_t requiredMemory, size_t subsysMemoryRequirement);

void ShutdownMemory();

GRAPI Allocator* GetGlobalAllocator();

// Copies memory from source to destination, also works if source and destination overlap
GRAPI void MemCopy(void* destination, void* source, size_t size);

GRAPI const u64 GetMemoryUsage();

GRAPI const u64 GetNetAllocations();

GRAPI void PrintMemoryStats();


#ifndef GR_DIST // These functions only get compiled if it's not a distribution build
GRAPI void AllocInfo(size_t size, mem_tag tag);

GRAPI void ReAllocInfo(i64 sizeChange);

GRAPI void FreeInfo(size_t size, mem_tag tag);
#endif // !GR_DIST