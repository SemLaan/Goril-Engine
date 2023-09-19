#include "allocator.h"
#include <core/asserts.h>
#include <core/gr_memory.h>



struct AllocHeader
{
	void* start;
	u32 size; // Size of the client allocation
	u32 alignment;
#ifndef GR_DIST
	mem_tag tag; // Only for debugging, gets optimized out of dist builds
#endif
};

size_t Allocator::GetAllocHeaderSize()
{
	return sizeof(AllocHeader);
}

void Allocator::Initialize(void* backendState, PFN_BackendAlloc allocPFN, PFN_BackendTryReAlloc tryReAllocPFN, PFN_BackendFree freePFN)
{
	this->backendState = backendState;
	BackendAlloc = allocPFN;
	BackendTryReAlloc = tryReAllocPFN;
	BackendFree = freePFN;
}

u32 Allocator::GetBlockSize(void* block)
{
	AllocHeader* header = (AllocHeader*)block - 1;
	return header->size;
}

void* Allocator::Alloc(size_t size, mem_tag tag)
{
	return AlignedAlloc(size, tag, MIN_ALIGNMENT);
}

void* Allocator::AlignedAlloc(size_t size, mem_tag tag, u32 alignment)
{
	// Checking if the alignment is greater than min alignment and is a power of two
	GRASSERT_DEBUG((alignment >= MIN_ALIGNMENT) && ((alignment & (alignment - 1)) == 0));

	u32 requiredSize = (u32)size + sizeof(AllocHeader) + alignment - 1;
#ifndef GR_DIST
	AllocInfo(requiredSize, tag);
#endif

	void* block = BackendAlloc(backendState, requiredSize);
	u64 blockExcludingHeader = (u64)block + sizeof(AllocHeader);
	// Gets the next address that is aligned on the requested boundary
	void* alignedBlock = (void*)((blockExcludingHeader + alignment - 1) & ~((u64)alignment - 1));

	// Putting in the header
	AllocHeader* header = (AllocHeader*)alignedBlock - 1;
	header->start = block;
	header->size = (u32)size;
	header->alignment = alignment;
#ifndef GR_DIST
	header->tag = tag; // Debug only
#endif
	// return the block to the client
	return alignedBlock;
}

void* Allocator::ReAlloc(void* block, size_t size)
{
	// Going slightly before the block and grabbing the alloc header that is stored there for debug info
	AllocHeader* header = (AllocHeader*)block - 1;
	GRASSERT(size != header->size);
	size_t newTotalSize = size + header->alignment - 1 + sizeof(AllocHeader);
	size_t oldTotalSize = header->size + header->alignment - 1 + sizeof(AllocHeader);

#ifndef GR_DIST
	ReAllocInfo((i64)newTotalSize - oldTotalSize);
#endif // !GR_DIST

	// ================== If the realloc is smaller than the original alloc ==========================
	// ===================== Or if there is enough space after the old alloc to just extend it ========================
	if (BackendTryReAlloc(backendState, header->start, oldTotalSize, newTotalSize))
	{
		header->size = (u32)size;
		return block;
	}

	// ==================== If there's no space at the old allocation ==========================================
	// ======================= Copy it to a new allocation and delete the old one ===============================
	// Get new allocation and align it
	void* newBlock = BackendAlloc(backendState, newTotalSize);
	u64 blockExcludingHeader = (u64)newBlock + sizeof(AllocHeader);
	void* alignedBlock = (void*)((blockExcludingHeader + header->alignment - 1) & ~((u64)header->alignment - 1));

	// Copy the client data
	MemCopy(alignedBlock, block, header->size);

	// Fill in the header at the new memory location
	AllocHeader* newHeader = (AllocHeader*)alignedBlock - 1;
	newHeader->start = newBlock;
	newHeader->size = (u32)size;
	newHeader->alignment = header->alignment;
#ifndef GR_DIST
	newHeader->tag = header->tag;
#endif // !GR_DIST

	// Free the old data
	BackendFree(backendState, header->start, oldTotalSize);

	return alignedBlock;
}

void Allocator::Free(void* block)
{
	// Going slightly before the block and grabbing the alloc header that is stored there for debug info
	AllocHeader* header = (AllocHeader*)block - 1;
	size_t totalFreeSize = header->size + header->alignment - 1 + sizeof(AllocHeader);

#ifndef GR_DIST
	FreeInfo(totalFreeSize, header->tag);
#endif

	BackendFree(backendState, header->start, totalFreeSize);
}
