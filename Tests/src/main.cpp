#include <goril.h>
#include "test_manager.h"
#include <iostream>
#include <core/application.h>

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

	// Init engine
	GR::GameConfig config = {};
	config.width = 0;
	config.height = 0;
	config.game_instance_memory_requirement = 1024;
	GR::InitializeEngine(config);

	// Run tests
	b8 result = run_tests();

	// Shutdown
	GR::ShutdownEngine();
	shutdown_test_manager();

	if (result == false)
	{
		GRFATAL("One or more tests failed!");
		std::cin.get();
	}

	return 0;
}