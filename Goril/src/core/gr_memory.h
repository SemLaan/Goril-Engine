#pragma once
#include "defines.h"

namespace GR
{
	struct GRAPI Blk
	{
		void* ptr;
		size_t length;
	};

	enum class GRAPI memory_tag
	{
		MEM_TAG_LINEAR_ALLOC,
		MAX_MEMORY_TAGS
	};

	// TODO: make some kind of engine setup in the test project so that initialize and shutdown memory don't have to be exported
	GRAPI b8 InitializeMemory(size_t arenaSize);

	GRAPI void ShutdownMemory();

	GRAPI Blk Alloc(size_t size, memory_tag tag);

	GRAPI void Free(Blk block);

	GRAPI void Zero(Blk block);

	GRAPI const size_t& GetMemoryUsage();
}