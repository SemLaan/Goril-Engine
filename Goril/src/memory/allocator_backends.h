#pragma once
#include "defines.h"
#include "allocator.h"

namespace GR
{
	b8 CreateGlobalAllocator(size_t arenaSize, Allocator* out_allocator, size_t* out_stateSize);
	void DestroyGlobalAllocator(Allocator allocator);

	// ================================== Freelist =========================================================================================================================================================
	Allocator CreateFreelistAllocator(size_t arenaSize);
	void DestroyFreelistAllocator(Allocator allocator);
	size_t FreelistGetFreeNodes(void* backendState);
}