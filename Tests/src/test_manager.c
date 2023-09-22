#include "test_manager.h"
#include "containers/darray.h"
#include "test_defines.h"
#include <core/timer.h>


static PFN_test* testsDarray;
static const char** test_namesDarray;
static Timer timer;


bool initialize_test_manager()
{
	testsDarray = DarrayCreate(sizeof(PFN_test), 100, GetGlobalAllocator(), MEM_TAG_TEST);
	test_namesDarray = DarrayCreate(sizeof(const char*), 100, GetGlobalAllocator(), MEM_TAG_TEST);
	StartOrResetTimer(&timer);

	return true;
}

void shutdown_test_manager()
{
}

void register_test(PFN_test test, const char* test_name)
{
	DarrayPushback(testsDarray, &test);
	DarrayPushback(test_namesDarray, &test_name);
}

bool run_tests()
{
	bool any_failed = false;

	TESTINFO("=================== Starting tests... ====================");

	for (u32 i = 0; i < DarrayGetSize(testsDarray); ++i)
	{
		TESTINFO("Running test [%u/%llu]: %s", i + 1, DarrayGetSize(testsDarray), test_namesDarray[i]);
		f64 test_start_time = TimerSecondsSinceStart(timer);
		bool result = testsDarray[i]();
		f64 test_end_time = TimerSecondsSinceStart(timer);
		if (result == false)
		{
			any_failed = true;
			GRERROR("Test unsuccessful!");
		}
		else
		{
			TESTINFO("Test succesfull, took: %.4fs", test_end_time - test_start_time);
		}
	}

	return any_failed == false;
}
