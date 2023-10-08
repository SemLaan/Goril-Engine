#include "test_manager.h"
#include <core/application.h>
#include "test_defines.h"
#include <platform/platform.h>
#include <core/event.h>

#include "core/memory_tests.h"
#include "core/event_tests.h"
#include "containers/container_tests.h"
#include "memory/allocator_tests.h"
#include "vector_intrinsics/vector_performance.h"
#include <stdio.h>
#include <stdlib.h>

static bool running = true;

bool QuitListener(EventCode code, EventData data)
{
	running = false;
	return false;
}

int main()
{
	// -------------- Init engine ------------------------
	GameConfig config = {};
	config.windowWidth = 1;
	config.windowHeight = 1;
	config.windowTitle = "Testing window";
	config.game_instance_memory_requirement = RESERVED_GAME_MEMORY;
	InitializeEngine(config);

	RegisterEventListener(EVCODE_QUIT, QuitListener);
	RegisterEventListener(EVCODE_KEY_DOWN, QuitListener);

	// Testing logging functions
	//GRFATAL("Testing log functions: %s", "succesfull");
	//GRERROR("Testing log functions: %s", "succesfull");
	//GRWARN("Testing log functions: %s", "succesfull");
	//GRINFO("Testing log functions: %s", "succesfull");
	//GRDEBUG("Testing log functions: %s", "succesfull");
	//GRTRACE("Testing log functions: %s", "succesfull");

	// Initing test manager
	if (!initialize_test_manager())
	{
		GRFATAL("Test manager somehow failed to init lol");
		char* temp = malloc(100);
		scanf("%c", temp);
		return 1;
	}

	// ----------------- register all tests ----------------
	register_memory_tests();
	register_allocator_tests();
	register_container_tests();
	register_event_tests();
	register_vector_perf_tests();
	// ----------------------------------------------------

	// Run tests
	bool result = run_tests();

	if (result == false)
	{
		GRFATAL("One or more tests failed!");
	}
	else
	{
		GRINFO("=============== All tests succesfull ==============");
	}

	while (running)
	{
		PlatformProcessMessage();
	}

	// Shutdown
	ShutdownEngine();
	shutdown_test_manager();

	return 0;
}
