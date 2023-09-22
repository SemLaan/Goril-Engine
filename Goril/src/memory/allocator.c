#include "allocator.h"
#include <core/asserts.h>
#include <core/gr_memory.h>



typedef struct AllocHeader
{
	void* start;
	u32 size; // Size of the client allocation
	u32 alignment;
#ifndef GR_DIST
	mem_tag tag; // Only for debugging, gets optimized out of dist builds
#endif
} AllocHeader;



u32 GetAllocHeaderSize()
{
	return sizeof(AllocHeader);
}

u64 GetBlockSize(void* block)
{
	AllocHeader* header = (AllocHeader*)block - 1;
	return header->size;
}

void* AlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment)
{
	// Checking if the alignment is greater than min alignment and is a power of two
	GRASSERT_DEBUG((alignment >= MIN_ALIGNMENT) && ((alignment & (alignment - 1)) == 0));

	u32 requiredSize = (u32)size + sizeof(AllocHeader) + alignment - 1;
#ifndef GR_DIST
	AllocInfo(requiredSize, tag);
#endif

	void* block = allocator->BackendAlloc(allocator->backendState, requiredSize);
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

void* ReAlloc(Allocator* allocator, void* block, u64 size)
{
	// Going slightly before the block and grabbing the alloc header that is stored there for debug info
	AllocHeader* header = (AllocHeader*)block - 1;
	GRASSERT(size != header->size);
	u64 newTotalSize = size + header->alignment - 1 + sizeof(AllocHeader);
	u64 oldTotalSize = header->size + header->alignment - 1 + sizeof(AllocHeader);

#ifndef GR_DIST
	ReAllocInfo((i64)newTotalSize - oldTotalSize);
#endif // !GR_DIST

	// ================== If the realloc is smaller than the original alloc ==========================
	// ===================== Or if there is enough space after the old alloc to just extend it ========================
	if (allocator->BackendTryReAlloc(allocator->backendState, header->start, oldTotalSize, newTotalSize))
	{
		header->size = (u32)size;
		return block;
	}

	// ==================== If there's no space at the old allocation ==========================================
	// ======================= Copy it to a new allocation and delete the old one ===============================
	// Get new allocation and align it
	void* newBlock = allocator->BackendAlloc(allocator->backendState, newTotalSize);
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
	allocator->BackendFree(allocator->backendState, header->start, oldTotalSize);

	return alignedBlock;
}

void Free(Allocator* allocator, void* block)
{
	// Going slightly before the block and grabbing the alloc header that is stored there for debug info
	AllocHeader* header = (AllocHeader*)block - 1;
	u64 totalFreeSize = header->size + header->alignment - 1 + sizeof(AllocHeader);

#ifndef GR_DIST
	FreeInfo(totalFreeSize, header->tag);
#endif

	allocator->BackendFree(allocator->backendState, header->start, totalFreeSize);
}
