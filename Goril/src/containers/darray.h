#pragma once
#include "defines.h"
#include "core/gr_memory.h"


// This makes sure arrays are cache line aligned on most consumer hardware
#define DARRAY_MIN_ALIGNMENT 64

// Scaling factor for when a darray doesn't have enough memory anymore
#define DARRAY_SCALING_FACTOR 1.6f



GRAPI void* DarrayCreate(u32 stride, u32 capacity, Allocator* allocator, mem_tag tag);
GRAPI void* DarrayCreateWithSize(u32 stride, u32 capacityAndCapacity, Allocator* allocator, mem_tag tag);

GRAPI void* DarrayPushback(void* elements, void* element);

GRAPI void DarrayPop(void* elements);

GRAPI void DarrayPopAt(void* elements, u32 index);

GRAPI void DarrayDestroy(void* elements);

GRAPI u32 DarrayGetSize(void* elements);
GRAPI void DarraySetSize(void* elements, u32 size);

// Only use on small arrays because performance is poor, consider a hash map
GRAPI b8 DarrayContains(void* elements, void* element);