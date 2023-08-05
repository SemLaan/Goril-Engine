#include "test_manager.h"
#include <vector>

static std::vector<PFN_test> tests;
static gr::Timer timer;

b8 initialize_test_manager()
{
	tests = std::vector<PFN_test>();
	timer = gr::Timer();

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

	for (u32 i = 0; i < tests.size(); ++i)
	{
		b8 result = tests[i]();
		if (result == false)
		{
			any_failed = true;
		}
	}

	return any_failed == false;
}
