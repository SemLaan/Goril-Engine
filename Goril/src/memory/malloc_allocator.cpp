#include "malloc_allocator.h"
#include <corecrt_malloc.h>

namespace GR
{

	MallocAllocator::MallocAllocator()
	{
	}

	MallocAllocator::~MallocAllocator()
	{
	}

	bool MallocAllocator::Owns(Blk block)
	{
		// Since this is the fallback allocator we're gonna assume that you know what objects are owned by this one
		// because you know it's not owned by any other allocator
		// also keeping track of what the malloc allocator owns would only be possible by keeping a list of pointers which is very expensive
		return true;
	}

	Blk MallocAllocator::Alloc(size_t size, mem_tag tag)
	{
		AllocInfo(size, tag);
		return { malloc(size), size, tag };
	}

	void MallocAllocator::Free(Blk block)
	{
		FreeInfo(block);
		free(block.ptr);
	}
}