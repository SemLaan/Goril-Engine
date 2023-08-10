#pragma once
#include "core/gr_memory.h"

namespace GR
{

	// TODO: alignment
	class GRAPI Allocator
	{
	public:
		Allocator() = default;
		virtual ~Allocator() = default;

		// find a way to do this without virtual because it's slow and we're doing custom allocation for speed
		virtual bool Owns(Blk block) = 0;

		virtual Blk Alloc(size_t size, mem_tag tag) = 0;
		virtual void Free(Blk block) = 0;
	};
}