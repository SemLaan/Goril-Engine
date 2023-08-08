#include <goril.h>
#include "test_manager.h"
#include <iostream>

#include "core/timer_tests.h"
#include "core/memory_tests.h"

int main()
{
	if (!initialize_test_manager())
	{
		GRFATAL("Test manager somehow failed to init lol");
		std::cin.get();
		return 1;
	}

	// ----------------- register all tests ----------------
	register_timer_tests();
	register_memory_tests();
	// ------------------------------------------------------

	b8 result = run_tests();

	shutdown_test_manager();

	if (result == false)
	{
		GRFATAL("One or more tests failed!");
		std::cin.get();
	}

	return 0;
}