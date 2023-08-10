#pragma once
#include "allocator.h"


namespace GR
{

	class GRAPI FreelistAllocator : public Allocator
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
		FreelistAllocator(void* arena, size_t arenaSize, u32 nodeCount);
		~FreelistAllocator();

		// Returns how much memory should be allocated in order to have enough space for the actual arena to be equal to arenaSize
		static void GetRequiredNodesAndMemorySize(size_t arenaSize, size_t* nodeMemoryReq, u32* nodeCount);

		bool Owns(Blk block) override;

		Blk Alloc(size_t size, mem_tag tag) override;
		void Free(Blk block) override;

	private:
		Node* GetNodeFromPool();
		void ReturnNodeToPool(Node* node);
	};
}