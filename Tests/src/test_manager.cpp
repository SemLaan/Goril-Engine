#include "test_manager.h"
#include <vector>
#include "test_defines.h"

static std::vector<PFN_test> tests;
static GR::Timer timer;


b8 initialize_test_manager()
{
	tests = std::vector<PFN_test>();
	timer = GR::Timer();

	return true;
}

void shutdown_test_manager()
{
}

void register_test(PFN_test test)
{
	tests.push_back(test);
}

b8 run_tests()
{
	b8 any_failed = false;

	TESTINFO("Starting tests...");

	for (u32 i = 0; i < tests.size(); ++i)
	{
		TESTINFO("Running test [{}/{}]", i+1, tests.size());
		f64 test_start_time = timer.SecondsSinceStart();
		b8 result = tests[i]();
		f64 test_end_time = timer.SecondsSinceStart();
		if (result == false)
		{
			any_failed = true;
			GRERROR("Test unsuccessful!");
		}
		else
		{
			// the rounding and multiplication at the end of this line is to only show 3 places after the decimal
			TESTINFO("Test succesfull, took: {}s", std::round((test_end_time - test_start_time) * 1000) / 1000);
		}
	}

	return any_failed == false;
}
