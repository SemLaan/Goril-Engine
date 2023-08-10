#pragma once
#include "allocator.h"

namespace GR
{
	class MallocAllocator : public Allocator
	{
	public:
		MallocAllocator();
		~MallocAllocator();

		bool Owns(Blk block) override;

		Blk Alloc(size_t size, mem_tag tag) override;
		void Free(Blk block) override;
	};
}