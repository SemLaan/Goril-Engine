#pragma once
#include "defines.h"

namespace GR
{
	// Forward declaring allocator
	class FreelistAllocator;

	enum GRAPI mem_tag
	{
		LOCAL_ALLOCATOR,
		MEMORY_SUBSYS,
		LOGGING_SUBSYS,
		PLATFORM_SUBSYS,
		EVENT_SUBSYS,
		RENDERER_SUBSYS,
		GAME,
		TEST,
		MAX_MEMORY_TAGS
	};

	struct GRAPI Blk
	{
		void* ptr;
		size_t size;
		mem_tag tag;
	};

	b8 InitializeMemory(size_t arenaSize);

	void ShutdownMemory();

	GRAPI inline FreelistAllocator* GetGlobalAllocator();

	GRAPI inline void AllocInfo(size_t size, mem_tag tag);

	GRAPI inline void FreeInfo(Blk& block);

	GRAPI inline void Zero(Blk block);

	GRAPI inline const size_t& GetMemoryUsage();

	GRAPI inline const u64& GetNetAllocations();

	GRAPI void PrintMemoryStats();
}