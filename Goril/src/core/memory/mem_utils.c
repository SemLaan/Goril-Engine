#include "mem_utils.h"

#include <string.h>
#include "memory_subsys.h"

void MemCopy(void* destination, const void* source, size_t size)
{
	// TODO: change global allocator usage to a temp allocator
	// Checking if destination and source overlap
	if ((u8*)destination + size > (u8*)source && (u8*)destination < (u8*)source + size)
	{
		void* intermediateBlock = Alloc(GetGlobalAllocator(), size, MEM_TAG_MEMORY_SUBSYS);
		memcpy(intermediateBlock, source, size);
		memcpy(destination, intermediateBlock, size);
		Free(GetGlobalAllocator(), intermediateBlock);
	}
	else // If the blocks don't overlap just copy them
	{
		memcpy(destination, source, size);
	}
}

void ZeroMem(void* block, u64 size)
{
	memset(block, 0, size);
}

bool CompareMemory(void* a, void* b, u64 size)
{
	return 0 == memcmp(a, b, size);
}