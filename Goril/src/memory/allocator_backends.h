#pragma once
#include "defines.h"
#include "allocator.h"

// Space added to allocators if safety space is true
#define ALLOCATOR_EXTRA_HEADER_AND_ALIGNMENT_SPACE KiB

bool CreateGlobalAllocator(size_t arenaSize, Allocator* out_allocator, size_t* out_stateSize);
void DestroyGlobalAllocator(Allocator allocator);

// ================================== Freelist allocator =================================================================================================================================================
GRAPI Allocator CreateFreelistAllocator(size_t arenaSize, bool safetySpace);
GRAPI void DestroyFreelistAllocator(Allocator allocator);
GRAPI size_t FreelistGetFreeNodes(void* backendState);

// ==================================== Bump allocator ================================================================================================================================================
GRAPI Allocator CreateBumpAllocator(size_t arenaSize, bool safetySpace);
GRAPI void DestroyBumpAllocator(Allocator allocator);
