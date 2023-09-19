#include "darray.h"

#include "core/asserts.h"

typedef struct Darray
{
	Allocator* allocator;
	void* memoryBlock;
	u32        stride;
	u32        size;
	u32        capacity;
} Darray;


void* DarrayCreate(u32 stride, u32 capacity, Allocator* allocator, mem_tag tag)
{
	void* block = allocator->Alloc(DARRAY_STATE_SIZE + (capacity * stride), tag);

	// Find min aligned address after making sure darray state fits
	void* alignedBlock = (void*)(((u64)block + DARRAY_STATE_SIZE) & ~((u64)DARRAY_MIN_ALIGNMENT - 1));

	Darray* state = (Darray*)((u64)alignedBlock - sizeof(Darray));
	state->allocator = allocator;
	state->memoryBlock = block;
	state->stride = stride;
	state->size = 0;
	state->capacity = capacity;

	return alignedBlock;
}

void* DarrayCreateWithSize(u32 stride, u32 capacityAndSize, Allocator* allocator, mem_tag tag)
{
	void* block = allocator->Alloc(DARRAY_STATE_SIZE + capacityAndSize * stride, tag);

	// Find min aligned address after making sure darray state fits
	void* alignedBlock = (void*)(((u64)block + DARRAY_STATE_SIZE) & ~((u64)DARRAY_MIN_ALIGNMENT - 1));

	Darray* state = (Darray*)((u64)alignedBlock - sizeof(Darray));
	state->allocator = allocator;
	state->memoryBlock = block;
	state->stride = stride;
	state->size = capacityAndSize;
	state->capacity = capacityAndSize;

	return alignedBlock;
}

void DarrayPushback(void* elements, void* element)
{
	Darray* state = (Darray*)((u64)elements - sizeof(Darray));
	if (state->size >= state->capacity)
	{
		state->capacity = (u32)(state->capacity * DARRAY_SCALING_FACTOR + 1);
		state->memoryBlock = state->allocator->ReAlloc(state->memoryBlock, (size_t)(state->stride * state->capacity) + DARRAY_STATE_SIZE);
	}

	memcpy((u8*)elements + (state->stride * state->size), element, (size_t)state->stride);
	state->size++;
}

void DarrayPop(void* elements)
{
	Darray* state = (Darray*)((u64)elements - sizeof(Darray));
	state->size--;
}

void DarrayPopAt(void* elements, u32 index)
{
	Darray* state = (Darray*)((u64)elements - sizeof(Darray));

	u8* address = (u8*)elements + (index * state->stride);
	MemCopy(address, address + state->stride, (state->size - 1 - index) * state->stride);

	state->size--;
}

void DarrayDestroy(void* elements)
{
	Darray* state = (Darray*)((u64)elements - sizeof(Darray));
	state->allocator->Free(state->memoryBlock);
}

u32 DarrayGetSize(void* elements)
{
	Darray* state = (Darray*)((u64)elements - sizeof(Darray));
	return state->size;
}

void DarraySetSize(void* elements, u32 size)
{
	Darray* state = (Darray*)((u64)elements - sizeof(Darray));
	state->size = size;
}

b8 DarrayContains(void* elements, void* element)
{
	Darray* state = (Darray*)((u64)elements - sizeof(Darray));

	for (u32 i = 0; i < state->size; i++)
	{
		if (0 == memcmp((u8*)elements + (i * state->stride), element, state->stride))
			return true;
	}

	return false;
}
