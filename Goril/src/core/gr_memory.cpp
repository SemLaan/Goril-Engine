#include "gr_memory.h"

#include "logger.h"
#include "asserts.h"


#ifndef GR_DIST
static const char* memTagToText[mem_tag::MAX_MEMORY_TAGS] = {
	"ALLOCATOR STATE    ",
	"SUB ARENA          ",
	"MEMORY SUBSYS      ",
	"LOGGING SUBSYS     ",
	"PLATFORM SUBSYS    ",
	"EVENT SUBSYS       ",
	"RENDERER SUBSYS    ",
	"INPUT SUBSYS       ",
	"GAME               ",
	"TEST               ",
	"DARRAY             ",
	"VERTEX BUFFER      ",
	"INDEX BUFFER       ",
	"TEXTURE            ",
};
#endif // !GR_DIST

struct MemoryState
{
	Allocator globalAllocator;
	size_t arenaSize;
#ifndef GR_DIST
	size_t memorySubsystemStateSize;
	size_t allocated;
	size_t deferredMemory;
	u64 netAllocationCount;
	u32 perTagAllocCount[mem_tag::MAX_MEMORY_TAGS];
#endif // !GR_DIST
};

static MemoryState* state = nullptr;
static b8 initialized = false;

GlobalAllocators* g_Allocators = nullptr;

b8 InitializeMemory(size_t requiredMemory, size_t subsysMemoryRequirement)
{
	GRASSERT_DEBUG(state == nullptr); // If this fails it means init was called twice
	GRINFO("Initializing memory subsystem...");
	initialized = false;

	// Creating the global allocator and allocating all application memory
	Allocator globalAllocator;
	size_t globalAllocatorStateSize;
	if (!CreateGlobalAllocator(requiredMemory, &globalAllocator, &globalAllocatorStateSize))
	{
		GRFATAL("Creating global allocator failed");
		return false;
	}

	// Creating the memory state
	state = (MemoryState*)globalAllocator.Alloc(sizeof(MemoryState), mem_tag::MEM_TAG_MEMORY_SUBSYS);
	Zero(state, sizeof(MemoryState));
	initialized = true;

	state->globalAllocator = globalAllocator;
	state->arenaSize = requiredMemory + globalAllocatorStateSize;
#ifndef GR_DIST
	state->allocated = 0;
	state->deferredMemory = 0;
	state->memorySubsystemStateSize = sizeof(MemoryState) + globalAllocatorStateSize + globalAllocator.GetAllocHeaderSize();
	state->netAllocationCount = 0;
	for (u32 i = 0; i < mem_tag::MAX_MEMORY_TAGS; i++)
	{
		state->perTagAllocCount[i] = 0;
	}

	AllocInfo(state->memorySubsystemStateSize, MEM_TAG_MEMORY_SUBSYS);
	AllocInfo(0, MEM_TAG_ALLOCATOR_STATE);
#endif // !GR_DIST

	g_Allocators = (GlobalAllocators*)GRAlloc(sizeof(GlobalAllocators), MEM_TAG_MEMORY_SUBSYS);
	g_Allocators->temporaryAllocator = CreateBumpAllocator(KiB * 5); /// TODO: make configurable

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
		DestroyBumpAllocator(g_Allocators->temporaryAllocator);
		GRFree(g_Allocators);
	}

#ifndef GR_DIST
	// Removing all the allocation info from the state to print memory stats one last time for debugging 
	// This way the programmer can check if everything else in the application was freed by seeing if it prints 0 net allocations
	FreeInfo(state->memorySubsystemStateSize, MEM_TAG_MEMORY_SUBSYS);
	FreeInfo(0, MEM_TAG_ALLOCATOR_STATE);
	PrintMemoryStats();
#endif // !GR_DIST

	initialized = false;

	Allocator globalAllocator = state->globalAllocator;
	GRFree(state);

	// Return all the application memory back to the OS
	DestroyGlobalAllocator(globalAllocator);
}

Allocator* GetGlobalAllocator()
{
	return &state->globalAllocator;
}

void* GRAlloc(size_t size, mem_tag tag)
{
	return state->globalAllocator.AlignedAlloc(size, tag, MIN_ALIGNMENT);
}

void* GRAlignedAlloc(size_t size, mem_tag tag, u32 alignment)
{
	return state->globalAllocator.AlignedAlloc(size, tag, alignment);
}

void* GReAlloc(void* block, size_t size)
{
	return state->globalAllocator.ReAlloc(block, size);
}

void GRFree(void* block)
{
	state->globalAllocator.Free(block);
}

#ifndef GR_DIST // These functions only get compiled if it's not a distribution build
void AllocInfo(size_t size, mem_tag tag)
{
	if (!initialized)
		return;
	if (tag != MEM_TAG_SUB_ARENA)
		state->allocated += size;
	else
		state->deferredMemory += size;
	if (state->allocated > state->arenaSize)
		GRERROR("Allocating more memory than the application initially asked for, you should probably increase the amount of requested memory in the game config");
	state->netAllocationCount++;
	state->perTagAllocCount[tag]++;
}

void ReAllocInfo(i64 sizeChange)
{
	if (!initialized)
		return;
	state->allocated += sizeChange;
}

void FreeInfo(size_t size, mem_tag tag)
{
	if (!initialized)
		return;
	if (tag != MEM_TAG_SUB_ARENA)
		state->allocated -= size;
	else
		state->deferredMemory -= size;
	if (state->allocated < 0)
		GRFATAL("Somehow deallocated more memory than was allocated, very impressive and efficient use of memory");
	state->netAllocationCount--;
	state->perTagAllocCount[tag]--;
}
#endif GR_DIST

void Zero(void* block, size_t size)
{
	memset(block, 0, size);
}

void MemCopy(void* destination, void* source, size_t size)
{
	// Checking if destination and source overlap
	if ((u8*)destination + size > source && destination < (u8*)source + size)
	{
		void* intermediateBlock = GRAlloc(size, MEM_TAG_MEMORY_SUBSYS);
		memcpy(intermediateBlock, source, size);
		memcpy(destination, intermediateBlock, size);
		GRFree(intermediateBlock);
	}
	else // If the blocks don't overlap just copy them
	{
		memcpy(destination, source, size);
	}
}

#ifndef GR_DIST
const size_t& GetMemoryUsage()
{
	return state->allocated;
}

const u64& GetNetAllocations()
{
	return state->netAllocationCount;
}
#endif // !GR_DIST

const char* GetMemoryScaleString(size_t bytes, size_t& out_scale)
{
	if (bytes < KiB)
	{
		out_scale = 1;
		return "B";
	}
	else if (bytes < MiB)
	{
		out_scale = KiB;
		return "KiB";
	}
	else if (bytes < GiB)
	{
		out_scale = MiB;
		return "MiB";
	}
	else
	{
		out_scale = GiB;
		return "GiB";
	}
}

std::string GetMemoryAmountString(size_t bytes, size_t scale)
{
	return std::format("{:.2f}", (f32)bytes / scale);
}

void PrintMemoryStats()
{
#ifndef GR_DIST

	GRINFO("Printing memory stats:");
	std::string number1;
	std::string number2;
	const char* scaleString;
	size_t scale;
	scaleString = GetMemoryScaleString(state->deferredMemory, scale);
	number1 = GetMemoryAmountString(state->deferredMemory, scale);
	GRINFO("Memory deferred to local allocators: {}{}", number1, scaleString);
	scaleString = GetMemoryScaleString(state->arenaSize, scale);
	number1 = GetMemoryAmountString(state->allocated, scale);
	number2 = GetMemoryAmountString(state->arenaSize, scale);
	GRINFO("Total allocated memory and total arena size ({}): {}/{}", scaleString, number1, number2);
	GRINFO("Percent allocated: {:.2f}%%", 100 * (f32)state->allocated / (f32)state->arenaSize);
	GRINFO("Total allocations: {}", state->netAllocationCount);
	GRINFO("Fragmentation (amount of separate free blocks): {}", FreelistGetFreeNodes(GetGlobalAllocator()->backendState));
	GRINFO("Allocations by tag:");
	for (u32 i = 0; i < mem_tag::MAX_MEMORY_TAGS; ++i)
	{
		GRINFO("	{}: {}", memTagToText[i], state->perTagAllocCount[i]);
	}
#endif // !GR_DIST
}


