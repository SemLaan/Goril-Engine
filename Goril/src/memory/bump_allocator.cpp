#include "bump_allocator.h"
#include "core/asserts.h"
#include "core/gr_memory.h"
#include "core/logger.h"

namespace GR
{
	struct AllocHeader // Only for debugging, gets optimized out of dist builds
	{
		u32 size;
#ifndef GR_DIST
		mem_tag tag;
#endif
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
		// Save metadata about the allocation just before the allocated block and bump the pointer to the place of the actual allocation
		AllocHeader* header = (AllocHeader*)m_bumpPointer;
		header->size = (u32)size + sizeof(AllocHeader);
#ifndef GR_DIST
		header->tag = tag;
		AllocInfo(header->size, tag);
#endif
		m_bumpPointer = (u8*)m_bumpPointer + sizeof(AllocHeader);

		// Allocating the actual block
		void* block = m_bumpPointer;
		m_bumpPointer = (u8*)m_bumpPointer + size;
		m_allocCount++;
		GRASSERT_MSG(m_bumpPointer <= m_arenaEnd, "Bump allocator overallocated");
		return block;
	}

	void* BumpAllocator::ReAlloc(void* block, size_t size)
	{
		// Going slightly before the block and grabbing the alloc header that is stored there for debug info
		AllocHeader* header = (AllocHeader*)block - 1;
		GRASSERT(size != (header->size - sizeof(AllocHeader)));

		// If the requested size is smaller than the allocated size just return block
		if ((header->size - sizeof(AllocHeader)) > size)
			return block;

		// If the memory in front of this block is free
		if ((u8*)block + (header->size - sizeof(AllocHeader)) == m_bumpPointer)
		{
			u32 newSize = (u32)size + sizeof(AllocHeader);
#ifndef GR_DIST
			ReAllocInfo(newSize - header->size);
#endif
			m_bumpPointer = (u8*)m_bumpPointer + (newSize - header->size);
			header->size = newSize;
			GRASSERT_MSG(m_bumpPointer <= m_arenaEnd, "Bump allocator overallocated");
			return block;
		}
		else // If the current memory needs to be freed and a new block needs to be allocated
		{
			GRWARN("Reallocating memory in bump allocator after other things have been allocated leaves wasted space in memory, you should probably find a different way to do what you're doing");
			// Free current block and alloc new block of requested size
			Free(block);
#ifndef GR_DIST // If not in distribution builds tag exists in header, otherwise it doesn't so we give a tag known at compile time
			return Alloc(size, header->tag);
#else
			return Alloc(size, MEM_TAG_LOCAL_ALLOCATOR);
#endif
		}
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
