#include "gr_memory.h"

#include "logger.h"
#include "asserts.h"
#include "memory/freelist_allocator.h"

namespace GR
{

	static const char* memTagToText[mem_tag::MAX_MEMORY_TAGS] = {
		"LOCAL_ALLOC        ",
		"MEMORY_SUBSYS      ",
		"LOGGING_SUBSYS     ",
		"PLATFORM_SUBSYS    ",
		"EVENT_SUBSYS       ",
		"RENDERER_SUBSYS    ",
		"GAME               ",
		"TEST               ",
		"DARRAY             ",
	};

	struct MemoryState
	{
		FreelistAllocator* globalAllocator;
		void* arenaBlock;
		size_t arenaSize;
		size_t allocated;
		u64 netAllocationCount;
		size_t memorySubsystemAllocSize;
		u32 perTagAllocCount[mem_tag::MAX_MEMORY_TAGS];
	};

	static MemoryState* state;
	static b8 initialized = false;

	b8 InitializeMemory(size_t requiredMemory)
	{
		initialized = false;

		// Getting the required memory for the memory subsystem state and the global allocator
		size_t totalArenaSize = requiredMemory + sizeof(MemoryState) + sizeof(FreelistAllocator);
		size_t freelistNodeMemory;
		u32 freelistNodeCount;
		FreelistAllocator::GetRequiredNodesAndMemorySize(totalArenaSize, &freelistNodeMemory, &freelistNodeCount);
		totalArenaSize += freelistNodeMemory;

		// Allocating all application memory
		void* arena = malloc(totalArenaSize);
		if (arena == nullptr)
		{
			GRFATAL("Couldn't allocate arena memory, initializing memory failed");
			return false;
		}

		// Creating the global allocator
		FreelistAllocator* allocator = (FreelistAllocator*)arena;
		// The allocator can see the entire arena, except for the memory where itself sits
#pragma warning( push ) // ==================================================== This pragma suppresses a warning about buffer overrun, the warning is obviously incorrect but
#pragma warning( disable : 6386 ) // ========================================== the code analysis is confused because i'm using malloc to allocate one big block of memory for the entire program
		allocator = new(allocator) FreelistAllocator();
		allocator->Initialize((u8*)arena + sizeof(FreelistAllocator), totalArenaSize - sizeof(FreelistAllocator), freelistNodeCount);
#pragma warning( pop )

		// Creating the memory state
		state = (MemoryState*)allocator->Alloc(sizeof(MemoryState), mem_tag::MEM_TAG_MEMORY_SUBSYS);
		*state = {};
		state->globalAllocator = allocator;
		state->arenaBlock = arena;
		state->arenaSize = totalArenaSize;
		state->allocated = 0;
		state->memorySubsystemAllocSize = sizeof(MemoryState) + sizeof(FreelistAllocator) + freelistNodeMemory + FreelistAllocator::GetAllocHeaderSize();
		state->netAllocationCount = 0;
		for (u32 i = 0; i < mem_tag::MAX_MEMORY_TAGS; i++)
		{
			state->perTagAllocCount[i] = 0;
		}

		initialized = true;

		AllocInfo(sizeof(MemoryState), MEM_TAG_MEMORY_SUBSYS);
		AllocInfo(sizeof(FreelistAllocator) + freelistNodeMemory + FreelistAllocator::GetAllocHeaderSize(), MEM_TAG_MEMORY_SUBSYS);

		return true;
	}

	void ShutdownMemory()
	{
		initialized = false;

		// Removing all the allocation info from the state to print memory stats one last time for debugging 
		// This way the programmer can check if everything else in the application was freed by seeing if it prints 0 net allocations
		FreeInfo(state->memorySubsystemAllocSize, MEM_TAG_MEMORY_SUBSYS);
		FreeInfo(0, MEM_TAG_MEMORY_SUBSYS);
		PrintMemoryStats();

		GetGlobalAllocator()->Free(state);
		// We dont have to destroy the global allocator because that does nothing

		// Return all the application memory back to the OS
		free(state->arenaBlock);
	}

	FreelistAllocator* GetGlobalAllocator()
	{
		return state->globalAllocator;
	}

	void AllocInfo(size_t size, mem_tag tag)
	{
		if (!initialized)
			return;
		state->allocated += size;
#ifndef GR_DIST
		if (state->allocated > state->arenaSize)
			GRERROR("Allocating more memory than the application initially asked for, you should probably increase the amount of requested memory in the game config");
		state->netAllocationCount++;
		state->perTagAllocCount[tag]++;
#endif
	}

	void FreeInfo(size_t size, mem_tag tag)
	{
		state->allocated -= size;
#ifndef GR_DIST
		if (state->allocated < 0)
			GRFATAL("Somehow deallocated more memory than was allocated, very impressive and efficient use of memory");
		state->netAllocationCount--;
		state->perTagAllocCount[tag]--;
#endif
	}

	void Zero(void* block, size_t size)
	{
		memset(block, 0, size);
	}

	void MemCopy(void* destination, void* source, size_t size)
	{
		memcpy(destination, source, size);
	}

	const size_t& GetMemoryUsage()
	{
		return state->allocated;
	}

	const u64& GetNetAllocations()
	{
		return state->netAllocationCount;
	}

	void PrintMemoryStats()
	{
		GRINFO("Printing memory stats:");
		GRINFO("Total allocated memory and total arena size (bytes): {}/{}", state->allocated, state->arenaSize);
		GRINFO("Total allocations: {}", state->netAllocationCount);
		GRINFO("Allocations by tag:");
		for (u32 i = 0; i < mem_tag::MAX_MEMORY_TAGS; ++i)
		{
			GRINFO("	{}: {}", memTagToText[i], state->perTagAllocCount[i]);
		}
	}
}

