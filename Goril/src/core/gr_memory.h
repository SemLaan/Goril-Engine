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

Allocator* GetGlobalAllocator();

// Copies memory from source to destination, also works if source and destination overlap
void MemCopy(void* destination, const void* source, size_t size);

void ZeroMem(void* block, u64 size);

bool CompareMemory(void* a, void* b, u64 size);

const u64 GetMemoryUsage();

const u64 GetNetAllocations();

void PrintMemoryStats();


#ifndef GR_DIST // These functions only get compiled if it's not a distribution build
void AllocInfo(size_t size, mem_tag tag);

void ReAllocInfo(i64 sizeChange);

void FreeInfo(size_t size, mem_tag tag);
#endif // !GR_DIST