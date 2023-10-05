#include "allocator_backends.h"

#include <core/asserts.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "core/gr_memory.h"

#define FREELIST_NODE_FACTOR 10

// All non-pool allocators store this in front of the user block
typedef struct AllocHeader
{
    void* start;
    u32 size; // Size of the client allocation
    u32 alignment;
#ifndef GR_DIST
    mem_tag tag; // Only for debugging, gets optimized out of dist builds
#endif
} AllocHeader;

// =====================================================================================================================================================================================================
// ================================== Freelist allocator ===============================================================================================================================================
// =====================================================================================================================================================================================================
typedef struct FreelistNode
{
    void* address;
    size_t size;
    struct FreelistNode* next;
} FreelistNode;

typedef struct FreelistState
{
    void* arenaStart;
    size_t arenaSize;
    FreelistNode* head;
    FreelistNode* nodePool;
    u32 nodeCount;
} FreelistState;

// These functions use other functions to do allocations and prepare the blocks for use
// (adding a header and aligning the block)
static void* FreelistAlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment);
static void* FreelistReAlloc(Allocator* allocator, void* block, u64 size);
static void FreelistFree(Allocator* allocator, void* block);

// These functions do actual allocation and freeing
static void* FreelistPrimitiveAlloc(void* backendState, size_t size);
static bool FreelistPrimitiveTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize);
static void FreelistPrimitiveFree(void* backendState, void* block, size_t size);

Allocator CreateFreelistAllocator(size_t arenaSize, bool safetySpace /*default: true*/)
{
    if (safetySpace)
        arenaSize += ALLOCATOR_EXTRA_HEADER_AND_ALIGNMENT_SPACE;

    // Calculating the required nodes for an arena of the given size
    // Make one node for every "freelist node factor" nodes that fit in the arena
    u32 nodeCount = (u32)(arenaSize / (FREELIST_NODE_FACTOR * sizeof(FreelistNode)));

    // Calculating required memory (client size + state size)
    size_t stateSize = sizeof(FreelistState) + nodeCount * sizeof(FreelistNode);
    size_t requiredMemory = arenaSize + stateSize;

    // Allocating memory for state and arena and zeroing state memory
    void* arenaBlock = Alloc(GetGlobalAllocator(), requiredMemory, MEM_TAG_SUB_ARENA);
    memset(arenaBlock, 0, stateSize);
#ifndef GR_DIST
    AllocInfo(stateSize, MEM_TAG_ALLOCATOR_STATE);
#endif // !GR_DIST

    // Getting pointers to the internal components of the allocator
    FreelistState* state = (FreelistState*)arenaBlock;
    FreelistNode* nodePool = (FreelistNode*)((u8*)arenaBlock + sizeof(FreelistState));
    void* arenaStart = (u8*)arenaBlock + stateSize;

    // Configuring allocator state
    state->arenaStart = arenaStart;
    state->arenaSize = arenaSize;
    state->head = nodePool;
    state->nodePool = nodePool;
    state->nodeCount = nodeCount;

    // Configuring head node
    state->head->address = arenaStart;
    state->head->size = arenaSize;
    state->head->next = nullptr;

    // Linking the allocator object to the freelist functions
    Allocator allocator = {};
    allocator.BackendAlloc = FreelistAlignedAlloc;
    allocator.BackendReAlloc = FreelistReAlloc;
    allocator.BackendFree = FreelistFree;
    allocator.backendState = state;

    return allocator;
}

void DestroyFreelistAllocator(Allocator allocator)
{
    FreelistState* state = (FreelistState*)allocator.backendState;
#ifndef GR_DIST
    FreeInfo(sizeof(FreelistState) + state->nodeCount * sizeof(FreelistNode), MEM_TAG_ALLOCATOR_STATE);
#endif // !GR_DIST

    // Frees the entire arena including state
    Free(GetGlobalAllocator(), state);
}

size_t FreelistGetFreeNodes(void* backendState)
{
    FreelistState* state = (FreelistState*)backendState;
    size_t count = 0;
    FreelistNode* node = state->head;

    while (node)
    {
        count++;
        node = node->next;
    }

    return count;
}

static FreelistNode* GetNodeFromPool(FreelistState* state)
{
    for (u32 i = 0; i < state->nodeCount; ++i)
    {
        if (state->nodePool[i].address == nullptr)
            return state->nodePool + i;
    }

    GRASSERT_MSG(false, "Ran out of pool nodes, decrease the x variable in the freelist allocator: get required memory size function. Or even better make more use of local allocators to avoid fragmentation");
    return nullptr;
}

static void ReturnNodeToPool(FreelistNode* node)
{
    node->address = nullptr;
    node->next = nullptr;
    node->size = 0;
}

static void* FreelistAlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment)
{
	// Checking if the alignment is greater than min alignment and is a power of two
    GRASSERT_DEBUG((alignment >= MIN_ALIGNMENT) && ((alignment & (alignment - 1)) == 0));

    u32 requiredSize = (u32)size + sizeof(AllocHeader) + alignment - 1;
#ifndef GR_DIST
    AllocInfo(requiredSize, tag);
#endif

    void* block = FreelistPrimitiveAlloc(allocator->backendState, requiredSize);
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

static void* FreelistReAlloc(Allocator* allocator, void* block, u64 size)
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
    if (FreelistPrimitiveTryReAlloc(allocator->backendState, header->start, oldTotalSize, newTotalSize))
    {
        header->size = (u32)size;
        return block;
    }

    // ==================== If there's no space at the old allocation ==========================================
    // ======================= Copy it to a new allocation and delete the old one ===============================
    // Get new allocation and align it
    void* newBlock = FreelistPrimitiveAlloc(allocator->backendState, newTotalSize);
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
    FreelistPrimitiveFree(allocator->backendState, header->start, oldTotalSize);

    return alignedBlock;
}

static void FreelistFree(Allocator* allocator, void* block)
{
	// Going slightly before the block and grabbing the alloc header that is stored there for debug info
    AllocHeader* header = (AllocHeader*)block - 1;
    u64 totalFreeSize = header->size + header->alignment - 1 + sizeof(AllocHeader);

#ifndef GR_DIST
    FreeInfo(totalFreeSize, header->tag);
#endif

    FreelistPrimitiveFree(allocator->backendState, header->start, totalFreeSize);
}

static void* FreelistPrimitiveAlloc(void* backendState, size_t size)
{
	FreelistState* state = (FreelistState*)backendState;
	FreelistNode* node = state->head;
	FreelistNode* previous = nullptr;

	while (node)
	{
		// If this node is the exact required size just use it
		if (node->size == size)
		{
			// Preparing the block to return to the client
			void* block = node->address;
			// Removing the node from the list and linking the list back together
			if (previous)
				previous->next = node->next;
			else // If the node is the head
				state->head = node->next;
			ReturnNodeToPool(node);
			return block;
		}
		// If this node is greater in size than requested, use it and split the node
		else if (node->size > size)
		{
			// Preparing the block to return to the client
			void* block = node->address;
			// Removing the now allocated memory from the node
			node->size -= size;
			node->address = (u8*)node->address + size;
			return block;
		}

		// If this node is smaller than the requested size, go to next node
		previous = node;
		node = node->next;
	}

	GRFATAL("Can't allocate object of size %llu", size);
	GRASSERT_MSG(false, "Freelist allocator ran out of memory or too fragmented");
	return nullptr;
}

static bool FreelistPrimitiveTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize)
{
	FreelistState* state = (FreelistState*)backendState;

	// ====== If the requested size is smaller than the allocated size just return block and free the leftovers of the block ==========================
	if (oldSize > newSize)
	{
		u32 freedSize = (u32)oldSize - (u32)newSize;
		FreelistPrimitiveFree(backendState, (u8*)block + oldSize, freedSize); // Freeing the memory
		return true;
	}
	else
	{
		// =================== If the requested size is bigger than our allocation ==============================
		// Trying to find a free node next in line that we can add to our allocation
		u32 requiredNodeSize = (u32)newSize - (u32)oldSize;
		void* requiredAddress = (u8*)block + oldSize;

		FreelistNode* node = state->head;
		FreelistNode* previous = nullptr;

		while (node)
		{
			// ==== If we find a free node right at the end of our allocation ================================
			if (node->address == requiredAddress)
			{
				if (node->size < requiredNodeSize)
					return false;
				// This if else statement updates the freelist
				if (node->size == requiredNodeSize)
				{
					if (previous)
						previous->next = node->next;
					else // If the node is the head
						state->head = node->next;
					ReturnNodeToPool(node);
				}
				else // If the node is not the exact required size
				{
					node->address = (u8*)node->address + requiredNodeSize;
					node->size -= requiredNodeSize;
				}
				return true;
			}
			else if (node->address > requiredAddress)
				return false;
			// If the block being reallocated sits after the current node, go to the next node
			previous = node;
			node = node->next;
		}
	}
	return false;
}

static void FreelistPrimitiveFree(void* backendState, void* block, size_t size)
{
	FreelistState* state = (FreelistState*)backendState;

	if (!state->head)
	{
		state->head = GetNodeFromPool(state);
		state->head->address = block;
		state->head->size = size;
		state->head->next = nullptr;
		return;
	}

	FreelistNode* node = state->head;
	FreelistNode* previous = nullptr;

	while (node || previous)
	{
		// If freed block sits before the current free node, or we're at the end of the list
		if ((node == nullptr) ? true : node->address > block)
		{
			// True if previous exists and end of previous aligns with start of freed block
			u8 aligns = previous ? ((u8*)previous->address + previous->size) == block : false;
			// True if the end of the freed block aligns with the start of the next node (also checks if node exist in case we are at the end of the list)
			aligns |= node ? (((u8*)block + size) == node->address) << 1 : false;

			// aligns:
			// 00 if nothing aligns
			// 01 if the previous aligns
			// 10 if the next aligns
			// 11 if both align

			FreelistNode* newNode = nullptr;

			switch (aligns)
			{
			case 0b00: // Nothing aligns ====================
				newNode = GetNodeFromPool(state);
				newNode->next = node;
				newNode->address = block;
				newNode->size = size;
				if (previous)
					previous->next = newNode;
				else
					state->head = newNode;
				return;
			case 0b01: // Previous aligns ===================
				previous->size += size;
				return;
			case 0b10: // Next aligns =======================
				node->address = block;
				node->size += size;
				return;
			case 0b11: // Previous and next align ===========
				previous->next = node->next;
				previous->size += size + node->size;
				ReturnNodeToPool(node);
				return;
			}
		}

		// If the block being freed sits after the current node, go to the next node
		previous = node;
		node = node->next;
	}

	GRASSERT_MSG(false, "I have no idea what went wrong, somehow the freelist free operation failed, good luck :)");
}

// =====================================================================================================================================================================================================
// ================================== Bump allocator ===================================================================================================================================================
// =====================================================================================================================================================================================================
typedef struct BumpAllocatorState
{
    void* arenaStart;
    void* bumpPointer;
    size_t arenaSize;
    u32 allocCount;
} BumpAllocatorState;

// These functions use other functions to do allocations and prepare the blocks for use
// (adding a header and aligning the block)
static void* BumpAlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment);
static void* BumpReAlloc(Allocator* allocator, void* block, u64 size);
static void BumpFree(Allocator* allocator, void* block);

// These functions do actual allocation and freeing
static void* BumpPrimitiveAlloc(void* backendState, size_t size);
static bool BumpPrimitiveTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize);
static void BumpPrimitiveFree(void* backendState, void* block, size_t size);

Allocator CreateBumpAllocator(size_t arenaSize, bool safetySpace /*default: true*/)
{
    if (safetySpace)
        arenaSize += ALLOCATOR_EXTRA_HEADER_AND_ALIGNMENT_SPACE;

    // Calculating required memory (client size + state size)
    size_t stateSize = sizeof(BumpAllocatorState);
    size_t requiredMemory = arenaSize + stateSize;

    // Allocating memory for state and arena and zeroing state memory
    void* arenaBlock = Alloc(GetGlobalAllocator(), requiredMemory, MEM_TAG_SUB_ARENA);
    memset(arenaBlock, 0, stateSize);
#ifndef GR_DIST
    AllocInfo(stateSize, MEM_TAG_ALLOCATOR_STATE);
#endif // !GR_DIST

    // Getting pointers to the internal components of the allocator
    BumpAllocatorState* state = (BumpAllocatorState*)arenaBlock;
    void* arenaStart = (u8*)arenaBlock + stateSize;

    // Configuring allocator state
    state->arenaStart = arenaStart;
    state->arenaSize = arenaSize;
    state->bumpPointer = arenaStart;
    state->allocCount = 0;

    // Linking the allocator object to the freelist functions
    Allocator allocator = {};
    allocator.BackendAlloc = BumpAlignedAlloc;
    allocator.BackendReAlloc = BumpReAlloc;
    allocator.BackendFree = BumpFree;
    allocator.backendState = state;

    return allocator;
}

void DestroyBumpAllocator(Allocator allocator)
{
    BumpAllocatorState* state = (BumpAllocatorState*)allocator.backendState;
#ifndef GR_DIST
    FreeInfo(sizeof(BumpAllocatorState), MEM_TAG_ALLOCATOR_STATE);
#endif // !GR_DIST

    // Frees the entire arena including state
    Free(GetGlobalAllocator(), state);
}

static void* BumpAlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment)
{
	// Checking if the alignment is greater than min alignment and is a power of two
    GRASSERT_DEBUG((alignment >= MIN_ALIGNMENT) && ((alignment & (alignment - 1)) == 0));

    u32 requiredSize = (u32)size + sizeof(AllocHeader) + alignment - 1;
#ifndef GR_DIST
    AllocInfo(requiredSize, tag);
#endif

    void* block = BumpPrimitiveAlloc(allocator->backendState, requiredSize);
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

static void* BumpReAlloc(Allocator* allocator, void* block, u64 size)
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
    if (BumpPrimitiveTryReAlloc(allocator->backendState, header->start, oldTotalSize, newTotalSize))
    {
        header->size = (u32)size;
        return block;
    }

    // ==================== If there's no space at the old allocation ==========================================
    // ======================= Copy it to a new allocation and delete the old one ===============================
    // Get new allocation and align it
    void* newBlock = BumpPrimitiveAlloc(allocator->backendState, newTotalSize);
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
    BumpPrimitiveFree(allocator->backendState, header->start, oldTotalSize);

    return alignedBlock;
}

static void BumpFree(Allocator* allocator, void* block)
{
	// Going slightly before the block and grabbing the alloc header that is stored there for debug info
    AllocHeader* header = (AllocHeader*)block - 1;
    u64 totalFreeSize = header->size + header->alignment - 1 + sizeof(AllocHeader);

#ifndef GR_DIST
    FreeInfo(totalFreeSize, header->tag);
#endif

    BumpPrimitiveFree(allocator->backendState, header->start, totalFreeSize);
}

static void* BumpPrimitiveAlloc(void* backendState, size_t size)
{
    BumpAllocatorState* state = (BumpAllocatorState*)backendState;

    // Allocating the actual block
    void* block = state->bumpPointer;
    state->bumpPointer = (u8*)state->bumpPointer + size;
    state->allocCount++;
    GRASSERT_MSG((u8*)state->bumpPointer <= ((u8*)state->arenaStart + state->arenaSize), "Bump allocator overallocated");
    return block;
}

static bool BumpPrimitiveTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize)
{
    BumpAllocatorState* state = (BumpAllocatorState*)backendState;

    if (oldSize > newSize)
    {
        return true;
    }
    else if (oldSize < newSize && ((u8*)block + oldSize) == state->bumpPointer)
    {
        state->bumpPointer = (u8*)state->bumpPointer + newSize - oldSize;
        return true;
    }
    return false;
}

static void BumpPrimitiveFree(void* backendState, void* block, size_t size)
{
    BumpAllocatorState* state = (BumpAllocatorState*)backendState;

    state->allocCount--;

    if (state->allocCount == 0)
    {
        state->bumpPointer = state->arenaStart;
    }
}

// =====================================================================================================================================================================================================
// ===================================== Pool allocator =============================================================================================================================================
// =====================================================================================================================================================================================================
typedef struct PoolAllocatorState
{
    void* poolStart;		// Pointer to the start of the memory that is managed by this allocator
    u32* controlBlocks;		// Pointer to the bitblocks that keep track of which blocks are free and which aren't
    u32 blockSize;			// Size of each block
    u32 poolSize;			// Amount of blocks in the pool
	u32 controlBlockCount;	// Amount of bitblocks in controlBlocks (each bitblock is a u32 that kan keep track of 32 blocks in the pool)
} PoolAllocatorState;

static void* PoolAlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment);
static void* PoolReAlloc(Allocator* allocator, void* block, u64 size);
static void PoolFree(Allocator* allocator, void* block);

Allocator CreatePoolAllocator(u32 blockSize, u32 poolSize)
{
    // Calculating required memory (client size + state size)
    u32 stateSize = sizeof(PoolAllocatorState);
    u32 blockTrackerSize = 4 * ceil((f32)poolSize / 32.f);
    u32 arenaSize = (blockSize * poolSize) + /*for alignment purposes*/ (blockSize - 1);
    u32 requiredMemory = arenaSize + stateSize + blockTrackerSize;

    // Allocating memory for state and arena and zeroing state memory
    void* arenaBlock = Alloc(GetGlobalAllocator(), requiredMemory, MEM_TAG_SUB_ARENA);
    memset(arenaBlock, 0, stateSize + blockTrackerSize);
#ifndef GR_DIST
    AllocInfo(stateSize + blockTrackerSize, MEM_TAG_ALLOCATOR_STATE);
#endif // !GR_DIST

    // Getting pointers to the internal components of the allocator
    PoolAllocatorState* state = (PoolAllocatorState*)arenaBlock;
    state->controlBlocks = (u32*)((u8*)arenaBlock + stateSize);
    state->poolStart = (void*)((u64)((state->controlBlocks + blockTrackerSize) + blockSize - 1) & ~((u64)blockSize - 1));

    // Configuring allocator state
    state->blockSize = blockSize;
    state->poolSize = poolSize;
	state->controlBlockCount = ceil((f32)poolSize / 32.f);

    // Linking the allocator object to the freelist functions
    Allocator allocator = {};
    allocator.BackendAlloc = PoolAlignedAlloc;
    allocator.BackendReAlloc = PoolReAlloc;
    allocator.BackendFree = PoolFree;
    allocator.backendState = state;

    return allocator;
}

void DestroyPoolAllocator(Allocator allocator)
{
    PoolAllocatorState* state = (PoolAllocatorState*)allocator.backendState;
#ifndef GR_DIST
    u32 stateSize = sizeof(PoolAllocatorState);
    u32 blockTrackerSize = 4 * state->controlBlockCount;
    FreeInfo(stateSize + blockTrackerSize, MEM_TAG_ALLOCATOR_STATE);
#endif // !GR_DIST

    // Frees the entire arena including state
    Free(GetGlobalAllocator(), state);
}

// From: http://tekpool.wordpress.com/category/bit-count/
u32 BitCount(u32 u)
{
	u32 uCount;

	uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

// This is used for finding the first free block in the pool quickly
u32 First0Bit(u32 i)
{
	i = ~i;
	return BitCount((i & (-i)) - 1);
}

static void* PoolAlignedAlloc(Allocator* allocator, u64 size, mem_tag tag, u32 alignment)
{
	PoolAllocatorState* state = (PoolAllocatorState*)allocator->backendState;

	GRASSERT_DEBUG(alignment == MIN_ALIGNMENT);
	GRASSERT_DEBUG(size == state->blockSize);

	u32 firstFreeBlock = UINT32_MAX;

	for (u32 i = 0; i < state->controlBlockCount; ++i)
	{
		if (state->controlBlocks[i] == UINT32_MAX)
			continue;
		
		u32 firstZeroBit = First0Bit(state->controlBlocks[i]);
		firstFreeBlock = (i * 32/*amount of bits in 32 bit int*/) + firstZeroBit;
		state->controlBlocks[i] |= 1 << firstZeroBit;
		break;
	}

	GRASSERT_MSG(firstFreeBlock < state->poolSize, "Pool allocator ran out of blocks");

	return (u8*)state->poolStart + (state->blockSize * firstFreeBlock);
}

static void* PoolReAlloc(Allocator* allocator, void* block, u64 size)
{
	GRASSERT_MSG(false, "Error, shit programmer detected!");
	return nullptr;
}

static void PoolFree(Allocator* allocator, void* block)
{
	PoolAllocatorState* state = (PoolAllocatorState*)allocator->backendState;

	u64 blockAddress = (u64)block;
	u64 poolStartAddress = (u64)state->poolStart;

	u64 relativeAddress = blockAddress - poolStartAddress;
	u64 poolBlockAddress = relativeAddress / state->blockSize;

	u32 controlBlockIndex = floor((f32)poolBlockAddress / 32.f);
	u32 bitAddress = poolBlockAddress % 32;

	// Setting the bit that manages the freed block to zero
	// Inverting the bits in the bitblock, then setting the bit to one, then inverting the block again
	state->controlBlocks[controlBlockIndex] = ~((1 << bitAddress) | (~state->controlBlocks[controlBlockIndex]));
}

// =====================================================================================================================================================================================================
// ================================== Global allocator creation =====================================================================
// =====================================================================================================================================================================================================
bool CreateGlobalAllocator(size_t arenaSize, Allocator* out_allocator, size_t* out_stateSize)
{
    // Calculating the required nodes for an arena of the given size
    // Make one node for every "freelist node factor" nodes that fit in the arena
    u32 nodeCount = (u32)(arenaSize / (FREELIST_NODE_FACTOR * sizeof(FreelistNode)));

    // Calculating required memory (client size + state size)
    size_t stateSize = sizeof(FreelistState) + nodeCount * sizeof(FreelistNode);
    *out_stateSize = stateSize;
    size_t requiredMemory = arenaSize + stateSize;

    // Allocating memory for state and arena and zeroing state memory
    void* arenaBlock = malloc(requiredMemory);
    if (arenaBlock == nullptr)
    {
        GRFATAL("Couldn't allocate arena memory, tried allocating %lluB, initializing memory failed", requiredMemory);
        return false;
    }

    memset(arenaBlock, 0, stateSize);

    // Getting pointers to the internal components of the allocator
    FreelistState* state = (FreelistState*)arenaBlock;
    FreelistNode* nodePool = (FreelistNode*)((u8*)arenaBlock + sizeof(FreelistState));
    void* arenaStart = (u8*)arenaBlock + stateSize;

    // Configuring allocator state
    state->arenaStart = arenaStart;
    state->arenaSize = arenaSize;
    state->head = nodePool;
    state->nodePool = nodePool;
    state->nodeCount = nodeCount;

    // Configuring head node
    state->head->address = arenaStart;
    state->head->size = arenaSize;
    state->head->next = nullptr;

    // Linking the allocator object to the freelist functions
    out_allocator->BackendAlloc = FreelistAlignedAlloc;
    out_allocator->BackendReAlloc = FreelistReAlloc;
    out_allocator->BackendFree = FreelistFree;
    out_allocator->backendState = state;

    return true;
}

void DestroyGlobalAllocator(Allocator allocator)
{
    FreelistState* state = (FreelistState*)allocator.backendState;
    // Frees the entire arena including state
    free(state);
}
