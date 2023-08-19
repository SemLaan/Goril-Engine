#include "freelist_allocator.h"
#include "core/asserts.h"
#include "core/gr_memory.h"

namespace GR
{
	
	struct AllocHeader
	{
		u32 size; // Size of the allocation including the size of the alloc header that is stored before the allocation
#ifndef GR_DIST
		mem_tag tag; // Only for debugging, gets optimized out of dist builds
#endif
	};

	void FreelistAllocator::Initialize(void* arena, size_t arenaSize, u32 nodeCount)
	{
		m_nodeCount = nodeCount;
		m_nodePool = (Node*)arena;

		for (u32 i = 0; i < m_nodeCount; ++i)
		{
			Node* current = m_nodePool + i;
			current = new(current) Node;
		}

		// The first address not used by the nodes of the linked list
		m_arenaStart = (Node*)arena + nodeCount;
		m_arenaEnd = (u8*)arena + arenaSize;

		m_head = m_nodePool;
		m_head->address = m_arenaStart;
		m_head->size = (u8*)m_arenaEnd - (u8*)m_arenaStart;
	}

	void FreelistAllocator::GetRequiredNodesAndMemorySize(size_t arenaSize, size_t* nodeMemoryReq, u32* nodeCount)
	{
		// Make one node for every x nodes that fit in the arena
		u32 x = 10;
		*nodeCount = (u32)((float)arenaSize / (float)((float)x * (float)sizeof(Node)));

		if (*nodeCount < 20)
			*nodeCount = 20;

		*nodeMemoryReq = *nodeCount * sizeof(Node);
	}

	size_t FreelistAllocator::GetAllocHeaderSize()
	{
		return sizeof(AllocHeader);
	}

	u32 FreelistAllocator::GetFreeNodes()
	{
		u32 count = 0;
		Node* node = m_head;

		while (node)
		{
			count++;
			node = node->next;
		}

		return count;
	}

	bool FreelistAllocator::Owns(void* block)
	{
		return (block >= m_arenaStart) && (block < m_arenaEnd);
	}

	void* FreelistAllocator::Alloc(size_t size, mem_tag tag)
	{
		u32 sizeWithHeader = (u32)size + sizeof(AllocHeader);

#ifndef GR_DIST
		AllocInfo(sizeWithHeader, tag);
#endif

		void* block = AllocInFreelist(sizeWithHeader);

		// Putting in the header
		AllocHeader* header = (AllocHeader*)block;
		header->size = sizeWithHeader;
#ifndef GR_DIST
		header->tag = tag; // Debug only
#endif
		// return the block to the client
		return (u8*)block + sizeof(AllocHeader);
	}

	void* FreelistAllocator::ReAlloc(void* block, size_t size)
	{
		// Going slightly before the block and grabbing the alloc header that is stored there for debug info
		AllocHeader* header = (AllocHeader*)block - 1;
		GRASSERT(size != (header->size - sizeof(AllocHeader)));
		size_t newTotalSize = size + sizeof(AllocHeader);

#ifndef GR_DIST
		ReAllocInfo((i64)newTotalSize - header->size);
#endif // !GR_DIST

		if (TryReAllocInFreelist(header, header->size, newTotalSize))
		{
			header->size = (u32)newTotalSize;
			return block;
		}

		// Else if realloc failed make new allocation and copy data in there
		void* newBlock = AllocInFreelist(newTotalSize);
		MemCopy(newBlock, header, header->size);
		FreeInFreelist(header, header->size);
		AllocHeader* newHeader = (AllocHeader*)newBlock;
		newHeader->size = (u32)newTotalSize;
		return (u8*)newBlock + sizeof(AllocHeader);
	}

	void FreelistAllocator::Free(void* block)
	{
		// Going slightly before the block and grabbing the alloc header that is stored there for debug info
		AllocHeader* header = (AllocHeader*)block - 1;
		void* freeAddress = header;

#ifndef GR_DIST
		FreeInfo(header->size, header->tag);
#endif

		FreeInFreelist(freeAddress, header->size);
	}

	void* FreelistAllocator::AllocInFreelist(size_t size)
	{
		Node* node = m_head;
		Node* previous = nullptr;

		while (node)
		{
			// If this node is the exact required size just use it
			if (node->size == size)
			{
				// Preparing the block to return to the client
				void* block = node->address;
				// Removing the node from the list and linking the list back together
				if (previous)
					previous->next = node->next;
				else // If the node is the head
					m_head = node->next;
				ReturnNodeToPool(node);
				return block;
			}
			// If this node is greater in size than requested, use it and split the node
			else if (node->size > size)
			{
				// Preparing the block to return to the client
				void* block = node->address;
				// Removing the now allocated memory from the node
				node->size -= size;
				node->address = (u8*)node->address + size;
				return block;
			}

			// If this node is smaller than the requested size, go to next node
			previous = node;
			node = node->next;
		}

		GRFATAL("Can't allocate object of size {}", size);
		GRASSERT_MSG(false, "Freelist allocator ran out of memory or too fragmented");
		return nullptr;
	}

	b8 FreelistAllocator::TryReAllocInFreelist(void* block, size_t oldSize, size_t newSize)
	{
		// ====== If the requested size is smaller than the allocated size just return block and free the leftovers of the block ==========================
		if (oldSize > newSize)
		{
			u32 freedSize = (u32)oldSize - (u32)newSize;
			FreeInFreelist((u8*)block + oldSize, freedSize); // Freeing the memory
			return true;
		}
		else
		{
			// =================== If the requested size is bigger than our allocation ==============================
			// Trying to find a free node next in line that we can add to our allocation
			u32 requiredNodeSize = (u32)newSize - (u32)oldSize;
			void* requiredAddress = (u8*)block + oldSize;

			Node* node = m_head;
			Node* previous = nullptr;

			while (node)
			{
				// ==== If we find a free node right at the end of our allocation ================================
				if (node->address == requiredAddress)
				{
					if (node->size < requiredNodeSize)
						return false;
					// This if else statement updates the freelist
					if (node->size == requiredNodeSize)
					{
						if (previous)
							previous->next = node->next;
						else // If the node is the head
							m_head = node->next;
						ReturnNodeToPool(node);
					}
					else // If the node is not the exact required size
					{
						node->address = (u8*)node->address + requiredNodeSize;
						node->size -= requiredNodeSize;
					}
					return true;
				}
				else if (node->address > requiredAddress)
					return false;
				// If the block being reallocated sits after the current node, go to the next node
				previous = node;
				node = node->next;
			}
		}
		return false;
	}

	void FreelistAllocator::FreeInFreelist(void* block, size_t size)
	{
		if (!m_head)
		{
			m_head = GetNodeFromPool();
			m_head->address = block;
			m_head->size = size;
			m_head->next = nullptr;
			return;
		}

		Node* node = m_head;
		Node* previous = nullptr;

		while (node || previous)
		{
			// If freed block sits before the current free node, or we're at the end of the list
			if ((node == nullptr) ? true : node->address > block)
			{
				// True if previous exists and end of previous aligns with start of freed block
				u8 aligns = previous ? ((u8*)previous->address + previous->size) == block : false;
				// True if the end of the freed block aligns with the start of the next node (also checks if node exist in case we are at the end of the list)
				aligns |= node ? (((u8*)block + size) == node->address) << 1 : false;

				// aligns:
				// 00 if nothing aligns
				// 01 if the previous aligns
				// 10 if the next aligns
				// 11 if both align

				Node* newNode = nullptr;

				switch (aligns)
				{
				case 0b00: // Nothing aligns ====================
					newNode = GetNodeFromPool();
					newNode->next = node;
					newNode->address = block;
					newNode->size = size;
					if (previous)
						previous->next = newNode;
					else
						m_head = newNode;
					return;
				case 0b01: // Previous aligns ===================
					previous->size += size;
					return;
				case 0b10: // Next aligns =======================
					node->address = block;
					node->size += size;
					return;
				case 0b11: // Previous and next align ===========
					previous->next = node->next;
					previous->size += size + node->size;
					ReturnNodeToPool(node);
					return;
				}
			}

			// If the block being freed sits after the current node, go to the next node
			previous = node;
			node = node->next;
		}

		GRASSERT_MSG(false, "I have no idea what went wrong, somehow the freelist free operation failed, good luck :)");
	}

	FreelistAllocator::Node* FreelistAllocator::GetNodeFromPool()
	{
		for (u32 i = 0; i < m_nodeCount; ++i)
		{
			if (m_nodePool[i].address == nullptr)
				return m_nodePool + i;
		}

		GRASSERT_MSG(false, "Ran out of pool nodes, decrease the x variable in the freelist allocator: get required memory size function. Or even better make more use of local allocators to avoid fragmentation");
		return nullptr;
	}

	void FreelistAllocator::ReturnNodeToPool(Node* node)
	{
		node->address = nullptr;
		node->next = nullptr;
		node->size = 0;
	}
}