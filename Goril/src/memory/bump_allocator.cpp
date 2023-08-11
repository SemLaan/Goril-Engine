#include "bump_allocator.h"
#include "core/asserts.h"
#include "core/gr_memory.h"

namespace GR
{
	struct AllocHeader // Only for debugging, gets optimized out of dist builds
	{
		u32 size;
		mem_tag tag;
	};

	void BumpAllocator::Initialize(void* arena, size_t arenaSize)
	{
		m_allocCount = 0;
		m_arenaEnd = (u8*)arena + arenaSize;
		m_arena = arena;
		m_bumpPointer = arena;
	}

	bool BumpAllocator::Owns(void* block)
	{
		return (block >= m_arena) && (block < m_arenaEnd);
	}

	size_t BumpAllocator::GetAllocHeaderSize()
	{
		return sizeof(AllocHeader);
	}

	void* BumpAllocator::Alloc(size_t size, mem_tag tag)
	{
#ifndef GR_DIST
		// Save metadata about the allocation just before the allocated block and bump the pointer to the place of the actual allocation
		AllocHeader* header = (AllocHeader*)m_bumpPointer;
		header->size = (u32)size + sizeof(AllocHeader);
		header->tag = tag;
		AllocInfo(header->size, tag);
		m_bumpPointer = (u8*)m_bumpPointer + sizeof(AllocHeader);
#endif
		// Allocating the actual block
		void* block = m_bumpPointer;
		m_bumpPointer = (u8*)m_bumpPointer + size;
		m_allocCount++;
		GRASSERT_MSG(m_bumpPointer <= m_arenaEnd, "Bump allocator overallocated");
		return block;
	}

	void BumpAllocator::Free(void* block)
	{
#ifndef GR_DIST
		// Going slightly before the block and grabbing the alloc header that is stored there for debug info
		AllocHeader* header = (AllocHeader*)block - 1;
		FreeInfo(header->size, header->tag);
#endif
		// Actual deallocation logic
		m_allocCount--;
		if (m_allocCount == 0)
		{
			m_bumpPointer = m_arena;
		}
	}
}
