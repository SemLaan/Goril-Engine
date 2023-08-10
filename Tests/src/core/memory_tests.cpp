#include "memory_tests.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <goril.h>


b8 alloc_and_dealloc_test()
{
	u64 initial_allocation_count = GR::GetNetAllocations();
	size_t test_allocation_size = 18;

	GR::Blk test_mem = GR::GetGlobalAllocator()->Alloc(test_allocation_size, GR::mem_tag::TEST);

	expect_to_be_true(test_mem.ptr != 0);
	expect_should_be(test_mem.size, test_allocation_size);
	expect_should_be(GR::GetNetAllocations() - initial_allocation_count, 1);

	GR::GetGlobalAllocator()->Free(test_mem);

	expect_should_be(GR::GetNetAllocations(), initial_allocation_count);

	return true;
}



void register_memory_tests()
{
	register_test(alloc_and_dealloc_test, "Global allocator: alloc and dealloc");
}