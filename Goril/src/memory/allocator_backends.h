#pragma once
#include "defines.h"
#include "allocator.h"

b8 CreateGlobalAllocator(size_t arenaSize, Allocator* out_allocator, size_t* out_stateSize);
void DestroyGlobalAllocator(Allocator allocator);

// ================================== Freelist allocator =================================================================================================================================================
GRAPI Allocator CreateFreelistAllocator(size_t arenaSize, b8 safetySpace = true);
GRAPI void DestroyFreelistAllocator(Allocator allocator);
GRAPI size_t FreelistGetFreeNodes(void* backendState);

// ==================================== Bump allocator ================================================================================================================================================
GRAPI Allocator CreateBumpAllocator(size_t arenaSize, b8 safetySpace = true);
GRAPI void DestroyBumpAllocator(Allocator allocator);
