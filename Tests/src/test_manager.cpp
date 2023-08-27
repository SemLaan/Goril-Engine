#include "test_manager.h"
#include <vector>
#include "test_defines.h"
#include <core/timer.h>

using namespace GR;

static std::vector<PFN_test> tests;
static std::vector<std::string> test_names;
static Timer timer;


b8 initialize_test_manager()
{
	tests = std::vector<PFN_test>();
	timer = CreateAndStartTimer();

	return true;
}

void shutdown_test_manager()
{
}

void register_test(PFN_test test, std::string test_name)
{
	tests.push_back(test);
	test_names.push_back(test_name);
}

b8 run_tests()
{
	b8 any_failed = false;

	TESTINFO("=================== Starting tests... ====================");

	for (u32 i = 0; i < tests.size(); ++i)
	{
		TESTINFO("Running test [{}/{}]: {}", i + 1, tests.size(), test_names[i]);
		f64 test_start_time = TimerSecondsSinceStart(timer);
		b8 result = tests[i]();
		f64 test_end_time = TimerSecondsSinceStart(timer);
		if (result == false)
		{
			any_failed = true;
			GRERROR("Test unsuccessful!");
		}
		else
		{
			TESTINFO("Test succesfull, took: {:.4f}s", test_end_time - test_start_time);
		}
	}

	return any_failed == false;
}
