#include "allocator_tests.h"
#include <memory/allocator_backends.h>

#include "test_defines.h"
#include "../test_manager.h"
#include <core/gr_memory.h>


static bool bump_allocator_test()
{
	size_t arena_size = 12 + Allocator::GetAllocHeaderSize() * 2 + MIN_ALIGNMENT * 2;

	Allocator allocator = CreateBumpAllocator(arena_size, false);

	void* temp = allocator.Alloc(8, MEM_TAG_TEST);
	void* temp2 = allocator.Alloc(4, MEM_TAG_TEST);
	allocator.Free(temp2);
	allocator.Free(temp);

	void* temp3 = allocator.Alloc(12, MEM_TAG_TEST);
	allocator.Free(temp3);

	DestroyBumpAllocator(allocator);

	return true;
}

static bool bump_allocator_realloc_test()
{
	size_t arena_size = 1000 + Allocator::GetAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateBumpAllocator(arena_size, false);

	void* temp = allocator.Alloc(700, MEM_TAG_TEST);
	temp = allocator.ReAlloc(temp, 800);
	allocator.Free(temp);

	void* temp4 = allocator.Alloc(1000, MEM_TAG_TEST);
	allocator.Free(temp4);

	DestroyBumpAllocator(allocator);

	return true;
}


static bool freelist_allocator_test()
{
	size_t arena_size = 1000 + Allocator::GetAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateFreelistAllocator(arena_size, false);

	// Testing full allocation and deallocation
	void* temp = allocator.Alloc(1000, MEM_TAG_TEST);
	allocator.Free(temp);

	expect_should_be(1, FreelistGetFreeNodes(allocator.backendState));

	// Testing mixed allocs and deallocs
	void* temp0 = allocator.Alloc(200, MEM_TAG_TEST);
	void* temp1 = allocator.Alloc(300, MEM_TAG_TEST);
	void* temp2 = allocator.Alloc(300, MEM_TAG_TEST);
	expect_should_be(300, allocator.GetBlockSize(temp2));
	void* temp3 = allocator.Alloc(80, MEM_TAG_TEST);
	allocator.Free(temp1);
	allocator.Free(temp2);
	expect_should_be(2, FreelistGetFreeNodes(allocator.backendState));
	void* temp4 = allocator.Alloc(500, MEM_TAG_TEST); // This will only allocate if it has properly combined free elements that are next to each other
	allocator.Free(temp0);
	allocator.Free(temp3);
	allocator.Free(temp4);

	temp = allocator.Alloc(1000, MEM_TAG_TEST);
	allocator.Free(temp);

	DestroyFreelistAllocator(allocator);

	return true;
}

static bool freelist_allocator_realloc_test()
{
	size_t arena_size = 1000 + Allocator::GetAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateFreelistAllocator(arena_size, false);

	void* temp = allocator.Alloc(700, MEM_TAG_TEST);
	temp = allocator.ReAlloc(temp, 800);
	expect_should_be(800, allocator.GetBlockSize(temp));
	allocator.Free(temp);

	void* temp4 = allocator.Alloc(1000, MEM_TAG_TEST);
	allocator.Free(temp4);


	void* temp1 = allocator.Alloc(200, MEM_TAG_TEST);
	void* temp2 = allocator.Alloc(200, MEM_TAG_TEST);
	temp1 = allocator.ReAlloc(temp1, 300);
	expect_should_be(300, allocator.GetBlockSize(temp1));
	allocator.Free(temp1);
	allocator.Free(temp2);

	void* temp3 = allocator.Alloc(1000, MEM_TAG_TEST);
	allocator.Free(temp3);

	void* temp5 = allocator.Alloc(800, MEM_TAG_TEST);
	temp5 = allocator.ReAlloc(temp5, 200);
	expect_should_be(200, allocator.GetBlockSize(temp5));
	void* temp6 = allocator.Alloc(500, MEM_TAG_TEST);
	allocator.Free(temp5);
	allocator.Free(temp6);

	DestroyFreelistAllocator(allocator);

	return true;
}

static bool freelist_allocator_alignment_test()
{
	size_t arena_size = 1000 + Allocator::GetAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateFreelistAllocator(arena_size, false);

	void* temp = allocator.AlignedAlloc(200, MEM_TAG_TEST, 8);
	expect_should_be(0, (u64)temp & 7);
	void* temp1 = allocator.AlignedAlloc(200, MEM_TAG_TEST, 64);
	expect_should_be(0, (u64)temp1 & 63);
	temp = allocator.ReAlloc(temp, 300);
	expect_should_be(0, (u64)temp & 7);
	allocator.Free(temp1);
	allocator.Free(temp);

	void* temp4 = allocator.Alloc(1000, MEM_TAG_TEST);
	allocator.Free(temp4);

	DestroyFreelistAllocator(allocator);

	return true;
}


void register_allocator_tests()
{
	register_test(bump_allocator_test, "Allocators: Bump allocator");
	register_test(bump_allocator_realloc_test, "Allocators: Bump allocator realloc");
	register_test(freelist_allocator_test, "Allocators: Freelist allocator");
	register_test(freelist_allocator_realloc_test, "Allocators: Freelist allocator realloc");
	register_test(freelist_allocator_alignment_test, "Allocators: Freelist alignment");
}
