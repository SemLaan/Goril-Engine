#pragma once
#include "defines.h"
#include "allocator_frontends.h"

typedef struct GlobalAllocators
{
	Allocator temporary;
} GlobalAllocators;

extern GlobalAllocators* g_Allocators;

bool InitializeMemory(size_t requiredMemory, size_t subsysMemoryRequirement);

void ShutdownMemory();

Allocator* GetGlobalAllocator();


const u64 GetMemoryUsage();

const u64 GetNetAllocations();

void PrintMemoryStats();


#ifndef GR_DIST // These functions only get compiled if it's not a distribution build
void AllocInfo(size_t size, MemTag tag);

void ReAllocInfo(i64 sizeChange);

void FreeInfo(size_t size, MemTag tag);
#endif // !GR_DIST