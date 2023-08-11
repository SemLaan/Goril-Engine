#pragma once
#include "defines.h"
#include "core/gr_memory.h"

namespace GR
{
	class MallocAllocator
	{
	public:
		MallocAllocator();
		~MallocAllocator();

		bool Owns(Blk block);

		Blk Alloc(size_t size, mem_tag tag);
		void Free(Blk block);
	};
}