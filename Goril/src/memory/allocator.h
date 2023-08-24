#pragma once
#include "defines.h"


#define ALLOCATOR_EXTRA_HEADER_AND_ALIGNMENT_SPACE KiB

namespace GR
{
	// Forward declaring mem_tag enum
	enum mem_tag;

	typedef void* (*PFN_BackendAlloc)(void* backendState, size_t size);
	typedef b8 (*PFN_BackendTryReAlloc)(void* backendState, void* block, size_t oldSize, size_t newSize);
	typedef void (*PFN_BackendFree)(void* backendState, void* block, size_t size);



	class GRAPI Allocator
	{
	private:
		PFN_BackendAlloc BackendAlloc;
		PFN_BackendTryReAlloc BackendTryReAlloc;
		PFN_BackendFree BackendFree;
	public:
		// Not supposed to be interacted with by the client
		void* backendState;

		// Not supposed to be called by client, use Create_name_Allocator functions
		void Initialize(void* backendState, PFN_BackendAlloc allocPFN, PFN_BackendTryReAlloc tryReAllocPFN, PFN_BackendFree freePFN);

		static size_t GetAllocHeaderSize();
		u32 GetBlockSize(void* block);

		void* Alloc(size_t size, mem_tag tag);
		void* AlignedAlloc(size_t size, mem_tag tag, u32 alignment);
		void* ReAlloc(void* block, size_t size);
		void Free(void* block);
	};
}