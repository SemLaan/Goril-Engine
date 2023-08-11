#pragma once
#include "defines.h"
#include "core/gr_memory.h"

namespace GR
{
	class GRAPI BumpAllocator
	{
	private:
		u32 m_allocCount;
		void* m_arenaEnd;
		void* m_arena;
		void* m_bumpPointer;
	public:
		BumpAllocator(void* arena, size_t arenaSize);
		~BumpAllocator();

		bool Owns(Blk block);

		Blk Alloc(size_t size, mem_tag tag);
		void Free(Blk block);
	};
}