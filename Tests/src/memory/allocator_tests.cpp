#include "allocator_tests.h"
#include <memory/bump_allocator.h>
#include <memory/freelist_allocator.h>

#include "test_defines.h"
#include "../test_manager.h"
#include <goril.h>

using namespace GR;

b8 bump_allocator_test()
{
	size_t arena_size = 12 + BumpAllocator::GetAllocHeaderSize() * 2;

	void* arena = malloc(arena_size);
	BumpAllocator* allocator = new BumpAllocator();
	allocator->Initialize(arena, arena_size);

	void* temp = allocator->Alloc(8, mem_tag::TEST);
	void* temp2 = allocator->Alloc(4, mem_tag::TEST);
	allocator->Free(temp2);
	allocator->Free(temp);

	void* temp3 = allocator->Alloc(12, mem_tag::TEST);
	allocator->Free(temp3);

	delete allocator;

	free(arena);

	return true;
}


b8 freelist_allocator_test()
{
	size_t arena_size = 1000;
	size_t required_node_memory;
	u32 required_nodes;

	FreelistAllocator::GetRequiredNodesAndMemorySize(arena_size, &required_node_memory, &required_nodes);
	arena_size += required_node_memory;

	void* arena = malloc(arena_size);
	FreelistAllocator* allocator = new FreelistAllocator();
	allocator->Initialize(arena, arena_size, required_nodes);

	// Testing full allocation and deallocation
	void* temp = allocator->Alloc(1000 - allocator->GetAllocHeaderSize(), mem_tag::TEST);
	allocator->Free(temp);

	// Testing mixed allocs and deallocs
	void* temp0 = allocator->Alloc(200, mem_tag::TEST);
	void* temp1 = allocator->Alloc(300, mem_tag::TEST);
	void* temp2 = allocator->Alloc(300, mem_tag::TEST);
	void* temp3 = allocator->Alloc(100, mem_tag::TEST);
	allocator->Free(temp1);
	allocator->Free(temp2);
	void* temp4 = allocator->Alloc(500, mem_tag::TEST); // This will only allocate if it has properly combined free elements that are next to each other
	allocator->Free(temp0);
	allocator->Free(temp3);
	allocator->Free(temp4);

	temp = allocator->Alloc(1000 - allocator->GetAllocHeaderSize(), mem_tag::TEST);
	allocator->Free(temp);

	delete allocator;

	free(arena);

	return true;
}


void register_allocator_tests()
{
	register_test(bump_allocator_test, "Allocators: Bump allocator");
	register_test(freelist_allocator_test, "Allocators: Freelist allocator");
}
