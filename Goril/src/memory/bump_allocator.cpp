#include "bump_allocator.h"
#include "core/asserts.h"

namespace GR
{
	BumpAllocator::BumpAllocator(void* arena, size_t arenaSize)
		: m_allocCount(0), m_arenaEnd((u8*)arena + arenaSize), m_arena(arena), m_bumpPointer(arena)
	{}

	BumpAllocator::~BumpAllocator()
	{

	}

	bool BumpAllocator::Owns(Blk block)
	{
		return (block.ptr >= m_arena) && (block.ptr < m_arenaEnd);
	}

	Blk BumpAllocator::Alloc(size_t size, mem_tag tag)
	{
		AllocInfo(size, tag);
		Blk block = { m_bumpPointer, size, tag };
		m_bumpPointer = (u8*)m_bumpPointer + size;
		m_allocCount++;
		GRASSERT_MSG(m_bumpPointer <= m_arenaEnd, "Bump allocator overallocated");
		return block;
	}

	void BumpAllocator::Free(Blk block)
	{
		m_allocCount--;
		FreeInfo(block);
		if (m_allocCount == 0)
		{
			m_bumpPointer = m_arena;
		}
	}
}
