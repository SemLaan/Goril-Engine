#include "memory_subsys.h"

#include "../logger.h"
#include "../asserts.h"
#include <string.h>



typedef struct MemoryState
{
	Allocator* globalAllocator;
	size_t arenaSize;
} MemoryState;

static MemoryState* state = nullptr;
static bool initialized = false;

GlobalAllocators* g_Allocators = nullptr;

bool InitializeMemory(size_t requiredMemory, size_t subsysMemoryRequirement)
{
	GRASSERT_DEBUG(state == nullptr); // If this fails it means init was called twice
	GRINFO("Initializing memory subsystem...");
	initialized = false;

	// Creating the global allocator and allocating all application memory
	Allocator* globalAllocator;
	size_t globalAllocatorStateSize;
	if (!CreateGlobalAllocator("Global allocator", requiredMemory, &globalAllocator, &globalAllocatorStateSize, nullptr))
	{
		GRFATAL("Creating global allocator failed");
		return false;
	}

	// Creating the memory state
	state = Alloc(globalAllocator, sizeof(MemoryState), MEM_TAG_MEMORY_SUBSYS);
	initialized = true;

	state->globalAllocator = globalAllocator;
	state->arenaSize = requiredMemory + globalAllocatorStateSize;

	g_Allocators = (GlobalAllocators*)Alloc(globalAllocator, sizeof(GlobalAllocators), MEM_TAG_MEMORY_SUBSYS);
	CreateBumpAllocator("Temporary allocator", globalAllocator, KiB * 5, &g_Allocators->temporary); /// TODO: make configurable

	return true;
}

void ShutdownMemory()
{
	if (state == nullptr)
	{
		GRINFO("Memory startup failed, skipping shutdown");
		return;
	}
	else
	{
		GRINFO("Shutting down memory subsystem...");
	}

	if (g_Allocators)
	{
		DestroyBumpAllocator(g_Allocators->temporary);
		Free(GetGlobalAllocator(), g_Allocators);
	}

	PRINT_MEMORY_STATS();

	initialized = false;

	Allocator* globalAllocator = state->globalAllocator;
	Free(GetGlobalAllocator(), state);

	// Return all the application memory back to the OS
	DestroyGlobalAllocator(globalAllocator);
}

Allocator* GetGlobalAllocator()
{
	return state->globalAllocator;
}

/*



void PrintMemoryStats()
{
#ifndef GR_DIST

	
	scaleString = GetMemoryScaleString(state->deferredMemory, &scale);
	GRINFO("Memory deferred to local allocators: %.2f%s", (f32)state->deferredMemory / scale, scaleString);
	scaleString = GetMemoryScaleString(state->arenaSize, &scale);
	GRINFO("Total allocated memory and total arena size (%s): %.2f/%.2f", scaleString, (f32)state->allocated / scale, (f32)state->arenaSize / scale);
	GRINFO("Percent allocated: %.2f%%", 100 * (f32)state->allocated / (f32)state->arenaSize);
	GRINFO("Total allocations: %llu", state->netAllocationCount);
	GRINFO("Fragmentation (amount of separate free blocks): %llu", FreelistGetFreeNodes(GetGlobalAllocator()->backendState));
	
#endif // !GR_DIST
}

*/
