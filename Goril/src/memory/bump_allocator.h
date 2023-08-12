#pragma once
#include "defines.h"

namespace GR
{
	// Forward declaring mem_tag enum
	enum mem_tag;

	class GRAPI BumpAllocator
	{
	private:
		u32 m_allocCount;
		void* m_arenaEnd;
		void* m_arena;
		void* m_bumpPointer;
	public:
		void Initialize(void* arena, size_t arenaSize);

		bool Owns(void* block);
		static size_t GetAllocHeaderSize();

		void* Alloc(size_t size, mem_tag tag);
		void* ReAlloc(void* block, size_t size);
		void Free(void* block);
	};
}