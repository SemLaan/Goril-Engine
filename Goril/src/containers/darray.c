#include "darray.h"

#include "core/asserts.h"
#include <string.h>

// NOTE: Darray size NEEDS to be smaller than DARRAY_MIN_ALIGNMENT
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
	GRASSERT_DEBUG(sizeof(Darray) < DARRAY_MIN_ALIGNMENT);

	void* block = AlignedAlloc(allocator, DARRAY_MIN_ALIGNMENT + (capacity * stride), tag, DARRAY_MIN_ALIGNMENT);

	void* elements = (u8*)block + DARRAY_MIN_ALIGNMENT;

	Darray* state = (Darray*)elements - 1;
	state->allocator = allocator;
	state->memoryBlock = block;
	state->stride = stride;
	state->size = 0;
	state->capacity = capacity;

	return elements;
}

void* DarrayCreateWithSize(u32 stride, u32 capacityAndSize, Allocator* allocator, mem_tag tag)
{
	void* block = AlignedAlloc(allocator, DARRAY_MIN_ALIGNMENT + (capacityAndSize * stride), tag, DARRAY_MIN_ALIGNMENT);

	void* elements = (u8*)block + DARRAY_MIN_ALIGNMENT;

	Darray* state = (Darray*)elements - 1;
	state->allocator = allocator;
	state->memoryBlock = block;
	state->stride = stride;
	state->size = capacityAndSize;
	state->capacity = capacityAndSize;

	return elements;
}

void* DarrayPushback(void* elements, void* element)
{
	Darray* state = (Darray*)elements - 1;
	if (state->size >= state->capacity)
	{
		state->capacity = (u32)(state->capacity * DARRAY_SCALING_FACTOR + 1);
		void* temp = ReAlloc(state->allocator, state->memoryBlock, (state->stride * state->capacity) + DARRAY_MIN_ALIGNMENT);
		elements = (u8*)temp + DARRAY_MIN_ALIGNMENT;
		state = (Darray*)elements - 1;
		state->memoryBlock = temp;
	}

	memcpy((u8*)elements + (state->stride * state->size), element, (size_t)state->stride);
	state->size++;
	return elements;
}

void DarrayPop(void* elements)
{
	Darray* state = (Darray*)elements - 1;
	state->size--;
}

void DarrayPopAt(void* elements, u32 index)
{
	Darray* state = (Darray*)elements - 1;

	u8* address = (u8*)elements + (index * state->stride);
	MemCopy(address, address + state->stride, (state->size - 1 - index) * state->stride);

	state->size--;
}

void DarrayDestroy(void* elements)
{
	Darray* state = (Darray*)elements - 1;
	Free(state->allocator, state->memoryBlock);
}

u32 DarrayGetSize(void* elements)
{
	Darray* state = (Darray*)elements - 1;
	return state->size;
}

void DarraySetSize(void* elements, u32 size)
{
	Darray* state = (Darray*)elements - 1;
	state->size = size;
}

bool DarrayContains(void* elements, void* element)
{
	Darray* state = (Darray*)elements - 1;

	for (u32 i = 0; i < state->size; i++)
	{
		if (0 == memcmp((u8*)elements + (i * state->stride), element, state->stride))
			return true;
	}

	return false;
}
