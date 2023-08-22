#include "memory_tests.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <core/gr_memory.h>

using namespace GR;

static b8 alloc_and_dealloc_test()
{
	u64 initial_allocation_count = GetNetAllocations();
	size_t test_allocation_size = 18;
	size_t test_change_pos = test_allocation_size + GetGlobalAllocator()->GetAllocHeaderSize();

	void* test_mem = GRAlloc(test_allocation_size, MEM_TAG_TEST);
	expect_to_be_true(test_mem != nullptr);

	int* nextAlloc = (int*)GRAlloc(sizeof(int), MEM_TAG_TEST);
	expect_to_be_true(nextAlloc != nullptr);

	// Making sure that the allocator gives the right amount of memory by testing 
	// if we are in the block of the next allocation if we reach out of the bounds of out own block
	*nextAlloc = 64;
	expect_should_be(64, *nextAlloc);

	GRFree(test_mem);

	expect_should_be(GetNetAllocations() - initial_allocation_count, 1);

	GRFree(nextAlloc);

	expect_should_be(GetNetAllocations(), initial_allocation_count);

	return true;
}



void register_memory_tests()
{
	register_test(alloc_and_dealloc_test, "Global allocator: alloc and dealloc");
}