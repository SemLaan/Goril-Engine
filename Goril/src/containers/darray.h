#pragma once
#include "defines.h"
#include "core/gr_memory.h"


// This makes sure arrays are cache line aligned on most consumer hardware
#define DARRAY_MIN_ALIGNMENT 64

// Scaling factor for when a darray doesn't have enough memory anymore
#define DARRAY_SCALING_FACTOR 1.6f


// Creates a dynamic array
void* DarrayCreate(u32 stride, u32 capacity, Allocator* allocator, mem_tag tag);
void* DarrayCreateWithSize(u32 stride, u32 capacityAndCapacity, Allocator* allocator, mem_tag tag);

// Note that this invalidates the old elements pointer so be carefull when using the darray in 2 places
// the pointer might become invalidated
void* DarrayPushback(void* elements, void* element);

void DarrayPop(void* elements);

void DarrayPopAt(void* elements, u32 index);

void DarrayDestroy(void* elements);

u32 DarrayGetSize(void* elements);
void DarraySetSize(void* elements, u32 size);

// Only use on small arrays because performance is poor, consider a hash map
bool DarrayContains(void* elements, void* element);