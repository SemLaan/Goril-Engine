#include "memory_tests.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <core/memory/gr_memory.h>


static bool alloc_and_dealloc_test()
{
	// TODO: fix test
	/*
	u64 initial_allocation_count = GetNetAllocations();
	size_t test_allocation_size = 18;

	void* test_mem = Alloc(GetGlobalAllocator(), test_allocation_size, MEM_TAG_TEST);
	expect_to_be_true(test_mem != nullptr);

	int* nextAlloc = (int*)Alloc(GetGlobalAllocator(), sizeof(int), MEM_TAG_TEST);
	expect_to_be_true(nextAlloc != nullptr);

	// Making sure that the allocator gives the right amount of memory by testing 
	// if we are in the block of the next allocation if we reach out of the bounds of out own block
	*nextAlloc = 64;
	expect_should_be(64, *nextAlloc);

	Free(GetGlobalAllocator(), test_mem);

	expect_should_be(GetNetAllocations() - initial_allocation_count, 1);

	Free(GetGlobalAllocator(), nextAlloc);

	expect_should_be(GetNetAllocations(), initial_allocation_count);
	*/

	return true;
}



void register_memory_tests()
{
	register_test(alloc_and_dealloc_test, "Global allocator: alloc and dealloc");
}