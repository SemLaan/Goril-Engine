#pragma once
#include "allocator.h"

namespace GR
{
	class GRAPI BumpAllocator : public Allocator
	{
	private:
		u32 m_allocCount;
		void* m_arenaEnd;
		void* m_arena;
		void* m_bumpPointer;
	public:
		BumpAllocator(void* arena, size_t arenaSize);
		~BumpAllocator();

		bool Owns(Blk block) override;

		Blk Alloc(size_t size, mem_tag tag) override;
		void Free(Blk block) override;
	};
}