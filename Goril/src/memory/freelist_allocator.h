#pragma once
#include "defines.h"

namespace GR
{
	// Forward declaring mem_tag enum
	enum mem_tag;

	class GRAPI FreelistAllocator
	{
		// =============== private Freelist Node class ==================================================
		struct Node
		{
		public:
			Node()
			{
				address = nullptr;
				size = 0;
				next = nullptr;
			}
			void* address;
			size_t size;
			Node* next;
		};
	private:
		
		// ======================= The freelist itself ==================================================
	private:
		void* m_arenaStart; // The first address not used by the nodes of the linked list
		void* m_arenaEnd;
		Node* m_head;
		Node* m_nodePool;
		u32 m_nodeCount;
	public:
		void Initialize(void* arena, size_t arenaSize, u32 nodeCount);

		// Returns how much memory should be allocated in order to have enough space for the actual arena to be equal to arenaSize
		static void GetRequiredNodesAndMemorySize(size_t arenaSize, size_t* nodeMemoryReq, u32* nodeCount);
		static size_t GetAllocHeaderSize();
		u32 GetFreeNodes();

		bool Owns(void* block);

		void* Alloc(size_t size, mem_tag tag);
		void* AlignedAlloc(size_t size, mem_tag tag, u32 alignment);
		void* ReAlloc(void* block, size_t size);
		void Free(void* block);

	private:
		void* AllocInFreelist(size_t size);
		b8 TryReAllocInFreelist(void* block, size_t oldSize, size_t newSize);
		void FreeInFreelist(void* block, size_t size);
		Node* GetNodeFromPool();
		void ReturnNodeToPool(Node* node);
	};
} 