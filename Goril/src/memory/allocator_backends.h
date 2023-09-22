#pragma once
#include "defines.h"
#include "allocator.h"

// Space added to allocators if safety space is true
#define ALLOCATOR_EXTRA_HEADER_AND_ALIGNMENT_SPACE KiB

bool CreateGlobalAllocator(size_t arenaSize, Allocator* out_allocator, size_t* out_stateSize);
void DestroyGlobalAllocator(Allocator allocator);

// ================================== Freelist allocator =================================================================================================================================================
Allocator CreateFreelistAllocator(size_t arenaSize, bool safetySpace);
void DestroyFreelistAllocator(Allocator allocator);
size_t FreelistGetFreeNodes(void* backendState);

// ==================================== Bump allocator ================================================================================================================================================
Allocator CreateBumpAllocator(size_t arenaSize, bool safetySpace);
void DestroyBumpAllocator(Allocator allocator);
