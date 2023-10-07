#include "allocator_tests.h"
#include <core/memory/allocator_backends.h>

#include "test_defines.h"
#include "../test_manager.h"
#include <core/memory/gr_memory.h>


static bool bump_allocator_test()
{
	// TODO: remove the alloc header size once bump allocator doesn't use it anymore
	size_t arena_size = 12 + GetFreelistAllocHeaderSize() * 2 + MIN_ALIGNMENT * 2;

	Allocator allocator = CreateBumpAllocator(arena_size, false);

	void* temp = Alloc(&allocator, 8, MEM_TAG_TEST);
	void* temp2 = Alloc(&allocator, 4, MEM_TAG_TEST);
	Free(&allocator, temp2);
	Free(&allocator, temp);

	void* temp3 = Alloc(&allocator, 12, MEM_TAG_TEST);
	Free(&allocator, temp3);

	DestroyBumpAllocator(allocator);

	return true;
}

static bool bump_allocator_realloc_test()
{
	size_t arena_size = 1000 + GetFreelistAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateBumpAllocator(arena_size, false);

	void* temp = Alloc(&allocator, 700, MEM_TAG_TEST);
	temp = ReAlloc(&allocator, temp, 800);
	Free(&allocator, temp);

	void* temp4 = Alloc(&allocator, 1000, MEM_TAG_TEST);
	Free(&allocator, temp4);

	DestroyBumpAllocator(allocator);

	return true;
}


static bool freelist_allocator_test()
{
	size_t arena_size = 1000 + GetFreelistAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateFreelistAllocator(arena_size, false);

	// Testing full allocation and deallocation
	void* temp = Alloc(&allocator, 1000, MEM_TAG_TEST);
	Free(&allocator, temp);

	expect_should_be(1, FreelistGetFreeNodes(allocator.backendState));

	// Testing mixed allocs and deallocs
	void* temp0 = Alloc(&allocator, 200, MEM_TAG_TEST);
	void* temp1 = Alloc(&allocator, 300, MEM_TAG_TEST);
	void* temp2 = Alloc(&allocator, 300, MEM_TAG_TEST);
	expect_should_be(300, GetBlockSize(temp2));
	void* temp3 = Alloc(&allocator, 80, MEM_TAG_TEST);
	Free(&allocator, temp1);
	Free(&allocator, temp2);
	expect_should_be(2, FreelistGetFreeNodes(allocator.backendState));
	void* temp4 = Alloc(&allocator, 500, MEM_TAG_TEST); // This will only allocate if it has properly combined free elements that are next to each other
	Free(&allocator, temp0);
	Free(&allocator, temp3);
	Free(&allocator, temp4);

	temp = Alloc(&allocator, 1000, MEM_TAG_TEST);
	Free(&allocator, temp);

	DestroyFreelistAllocator(allocator);

	return true;
}

static bool freelist_allocator_realloc_test()
{
	size_t arena_size = 1000 + GetFreelistAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateFreelistAllocator(arena_size, false);

	void* temp = Alloc(&allocator, 700, MEM_TAG_TEST);
	temp = ReAlloc(&allocator, temp, 800);
	expect_should_be(800, GetBlockSize(temp));
	Free(&allocator, temp);

	void* temp4 = Alloc(&allocator, 1000, MEM_TAG_TEST);
	Free(&allocator, temp4);


	void* temp1 = Alloc(&allocator, 200, MEM_TAG_TEST);
	void* temp2 = Alloc(&allocator, 200, MEM_TAG_TEST);
	temp1 = ReAlloc(&allocator, temp1, 300);
	expect_should_be(300, GetBlockSize(temp1));
	Free(&allocator, temp1);
	Free(&allocator, temp2);

	void* temp3 = Alloc(&allocator, 1000, MEM_TAG_TEST);
	Free(&allocator, temp3);

	void* temp5 = Alloc(&allocator, 800, MEM_TAG_TEST);
	temp5 = ReAlloc(&allocator, temp5, 200);
	expect_should_be(200, GetBlockSize(temp5));
	void* temp6 = Alloc(&allocator, 500, MEM_TAG_TEST);
	Free(&allocator, temp5);
	Free(&allocator, temp6);

	DestroyFreelistAllocator(allocator);

	return true;
}

static bool freelist_allocator_alignment_test()
{
	size_t arena_size = 1000 + GetFreelistAllocHeaderSize() + MIN_ALIGNMENT;

	Allocator allocator = CreateFreelistAllocator(arena_size, false);

	void* temp = AlignedAlloc(&allocator, 200, MEM_TAG_TEST, 8);
	expect_should_be(0, (u64)temp & 7);
	void* temp1 = AlignedAlloc(&allocator, 200, MEM_TAG_TEST, 64);
	expect_should_be(0, (u64)temp1 & 63);
	temp = ReAlloc(&allocator, temp, 300);
	expect_should_be(0, (u64)temp & 7);
	Free(&allocator, temp1);
	Free(&allocator, temp);

	void* temp4 = Alloc(&allocator, 1000, MEM_TAG_TEST);
	Free(&allocator, temp4);

	DestroyFreelistAllocator(allocator);

	return true;
}



typedef struct PoolTester
{
	u32 a;
	u64 b;
} PoolTester;

#include <stdlib.h>
static bool pool_allocator_test()
{
	const u32 poolElementCount = 100;

	PoolTester** testElements = malloc(sizeof(*testElements) * poolElementCount);

	Allocator allocator = CreatePoolAllocator(sizeof(PoolTester), poolElementCount);

	for (u32 i = 0; i < poolElementCount; ++i)
	{
		testElements[i] = Alloc(&allocator, sizeof(PoolTester), MEM_TAG_TEST);
		testElements[i]->a = i;
		testElements[i]->b = i * 2;
	}

	for (u32 i = 0; i < poolElementCount; ++i)
	{
		expect_to_be_false(testElements[i]->a != i || testElements[i]->b != i * 2);
	}

	testElements[50]->a = 1;
	testElements[50]->b = 1;

	expect_to_be_false(testElements[49]->b != 49 * 2);
	expect_to_be_false(testElements[51]->a != 51);
	
	Free(&allocator, testElements[50]);

	testElements[70] = Alloc(&allocator, sizeof(PoolTester), MEM_TAG_TEST);

	testElements[70]->a = 15675;
	testElements[70]->b = 15745;

	expect_to_be_false(testElements[49]->b != 49 * 2);
	expect_to_be_false(testElements[51]->a != 51);

	for (u32 i = 0; i < poolElementCount; ++i)
	{
		if (i != 50)
			Free(&allocator, testElements[i]);
	}

	free(testElements);

	DestroyPoolAllocator(allocator);

	return true;
}


void register_allocator_tests()
{
	register_test(bump_allocator_test, "Allocators: Bump allocator");
	register_test(bump_allocator_realloc_test, "Allocators: Bump allocator realloc");
	register_test(freelist_allocator_test, "Allocators: Freelist allocator");
	register_test(freelist_allocator_realloc_test, "Allocators: Freelist allocator realloc");
	register_test(freelist_allocator_alignment_test, "Allocators: Freelist alignment");
	register_test(pool_allocator_test, "Allocators: pool allocator");
}
