#include "memory_tests.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <goril.h>


b8 alloc_and_dealloc_test()
{
	size_t initial_allocated = GR::GetMemoryUsage();
	size_t test_allocation_size = 18;

	GR::Blk test_mem = GR::Alloc(test_allocation_size, GR::memory_tag::MEM_TAG_LINEAR_ALLOC);

	expect_to_be_true(test_mem.ptr != 0);
	expect_should_be(test_mem.length, test_allocation_size);
	expect_should_be(GR::GetMemoryUsage() - initial_allocated, test_allocation_size);

	GR::Free(test_mem);

	expect_should_be(GR::GetMemoryUsage(), initial_allocated);

	return true;
}



void register_memory_tests()
{
	register_test(alloc_and_dealloc_test, "Global allocator: alloc and dealloc");
}