#include "allocator_backends.h"

#include "core/gr_memory.h"
#include <core/asserts.h>
#include <stdlib.h>
#include <string.h>

#define FREELIST_NODE_FACTOR 10


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

static void* FreelistAlloc(void* backendState, size_t size);
static bool FreelistTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize);
static void FreelistFree(void* backendState, void* block, size_t size);

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
	allocator.BackendAlloc = FreelistAlloc;
	allocator.BackendTryReAlloc = FreelistTryReAlloc;
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

static void* FreelistAlloc(void* backendState, size_t size)
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

static bool FreelistTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize)
{
	FreelistState* state = (FreelistState*)backendState;

	// ====== If the requested size is smaller than the allocated size just return block and free the leftovers of the block ==========================
	if (oldSize > newSize)
	{
		u32 freedSize = (u32)oldSize - (u32)newSize;
		FreelistFree(backendState, (u8*)block + oldSize, freedSize); // Freeing the memory
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

static void FreelistFree(void* backendState, void* block, size_t size)
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
	size_t arenaSize;
	void* bumpPointer;
	u32 allocCount;
} BumpAllocatorState;

static void* BumpAlloc(void* backendState, size_t size);
static bool BumpTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize);
static void BumpFree(void* backendState, void* block, size_t size);

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
	allocator.BackendAlloc = BumpAlloc;
	allocator.BackendTryReAlloc = BumpTryReAlloc;
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

static void* BumpAlloc(void* backendState, size_t size)
{
	BumpAllocatorState* state = (BumpAllocatorState*)backendState;

	// Allocating the actual block
	void* block = state->bumpPointer;
	state->bumpPointer = (u8*)state->bumpPointer + size;
	state->allocCount++;
	GRASSERT_MSG((u8*)state->bumpPointer <= ((u8*)state->arenaStart + state->arenaSize), "Bump allocator overallocated");
	return block;
}

static bool BumpTryReAlloc(void* backendState, void* block, size_t oldSize, size_t newSize)
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

static void BumpFree(void* backendState, void* block, size_t size)
{
	BumpAllocatorState* state = (BumpAllocatorState*)backendState;

	state->allocCount--;

	if (state->allocCount == 0)
	{
		state->bumpPointer = state->arenaStart;
	}
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
	out_allocator->BackendAlloc = FreelistAlloc;
	out_allocator->BackendTryReAlloc = FreelistTryReAlloc;
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
