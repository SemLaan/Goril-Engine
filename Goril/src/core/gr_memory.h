#pragma once
#include "defines.h"

namespace GR
{
	enum GRAPI memory_tag
	{
		MEM_TAG_LINEAR_ALLOC,
		MAX_MEMORY_TAGS
	};

	struct GRAPI Blk
	{
		void* ptr;
		size_t length;
		memory_tag tag;
	};

	b8 InitializeMemory(size_t arenaSize);

	void ShutdownMemory();

	GRAPI Blk Alloc(size_t size, memory_tag tag);

	GRAPI void Free(Blk block);

	GRAPI void Zero(Blk block);

	GRAPI const size_t& GetMemoryUsage();

	GRAPI void PrintMemoryStats();
}