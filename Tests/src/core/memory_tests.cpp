#include "memory_tests.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <goril.h>


b8 alloc_and_dealloc_test()
{
	u64 initial_allocation_count = GR::GetNetAllocations();
	size_t test_allocation_size = 18;
	size_t test_change_pos = test_allocation_size + GR::GetGlobalAllocator()->GetAllocHeaderSize();

	void* test_mem = GR::GetGlobalAllocator()->Alloc(test_allocation_size, GR::TEST);
	expect_to_be_true(test_mem != nullptr);

	int* nextAlloc = (int*)GR::GetGlobalAllocator()->Alloc(4, GR::TEST);
	expect_to_be_true(nextAlloc != nullptr);

	int* nextAllocCheat = (int*)((u8*)test_mem + test_change_pos);

	// Making sure that the allocator gives the right amount of memory by testing 
	// if we are in the block of the next allocation if we reach out of the bounds of out own block
	*nextAlloc = 0;
	expect_should_be(*nextAlloc, 0);

	*nextAllocCheat = 3;
	expect_should_be(*nextAlloc, 3);

	GR::GetGlobalAllocator()->Free(test_mem);

	expect_should_be(GR::GetNetAllocations() - initial_allocation_count, 1);

	GR::GetGlobalAllocator()->Free(nextAlloc);

	expect_should_be(GR::GetNetAllocations(), initial_allocation_count);

	return true;
}



void register_memory_tests()
{
	register_test(alloc_and_dealloc_test, "Global allocator: alloc and dealloc");
}