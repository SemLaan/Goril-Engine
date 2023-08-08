#include "gr_memory.h"

#include <vcruntime_string.h>
#include <corecrt_malloc.h>
#include "logger.h"
#include "asserts.h"

namespace GR
{

	struct MemoryState
	{
		void* arenaBlock;
		size_t arenaSize;
		size_t allocated;
		u64 netAllocationCount;
	};

	static MemoryState* state;

	b8 InitializeMemory(size_t requiredMemory)
	{
		// Allocating all application memory
		requiredMemory += sizeof(MemoryState);
		void* arena = malloc(requiredMemory);
		if (arena == nullptr)
		{
			GRFATAL("Couldn't allocate arena memory, initializing memory failed");
			return false;
		}

		// Using the first piece of memory for the memory subsystem state
		state = (MemoryState*)arena;
		state->arenaSize = requiredMemory;
		state->arenaBlock = arena;
		state->allocated = 0;
		state->allocated += sizeof(MemoryState);
		return true;
	}

	void ShutdownMemory()
	{
		free(state->arenaBlock);
	}

	Blk Alloc(size_t size, memory_tag tag)
	{
		// TODO: alignment
#ifndef GR_DIST
		state->netAllocationCount++;
#endif
		state->allocated += size;
		if (state->allocated > state->arenaSize)
			GRWARN("Tried to allocate more memory than is available in arena, falling back to malloc");
		// TODO: store file and line info (maybe, this would require macro's which is ugly)
		// TODO: implement general dynamic allocator
		return { malloc(size), size };
	}

	void Free(Blk block)
	{
#ifndef GR_DIST
		state->netAllocationCount--;
#endif
		state->allocated -= block.length;
		free(block.ptr);
	}

	void Zero(Blk block)
	{
		memset(block.ptr, 0, block.length);
	}

	const size_t& GetMemoryUsage()
	{
		return state->allocated;
	}
}

