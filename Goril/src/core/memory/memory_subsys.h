#pragma once
#include "defines.h"
#include "allocator_frontends.h"

typedef struct GlobalAllocators
{
	Allocator* temporary;
} GlobalAllocators;

extern GlobalAllocators* g_Allocators;

bool InitializeMemory(size_t requiredMemory, size_t subsysMemoryRequirement);

void ShutdownMemory();

Allocator* GetGlobalAllocator();
