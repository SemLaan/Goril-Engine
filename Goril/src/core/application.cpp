#include "application.h"

#include "logger.h"
#include "platform.h"
#include "gr_memory.h"

namespace GR
{
	b8 InitializeEngine(GameConfig config)
	{
		size_t engineMemoryRequirement = KiB * 5;
		size_t subsysAllocatorRequirement = KiB;

		// Initialize subsystems
		if (!InitializeMemory(config.game_instance_memory_requirement + engineMemoryRequirement, subsysAllocatorRequirement))
		{
			GRFATAL("Memory failed to initialize");
			return false;
		}
		if (!InitializeLogger())
		{
			GRFATAL("Logger failed to initialize");
			return false;
		}
		if (!InitializePlatform())
		{
			GRFATAL("Platform failed to initialize");
			return false;
		}

		PrintMemoryStats();

		return true;
	}

	b8 RunEngine(GorilGame* gameInstance)
	{
		gameInstance->Init();

		while (true)
		{
			PlatformProcessMessage();
			gameInstance->Update();
			gameInstance->Render();
		}

		gameInstance->Shutdown();

		return true;
	}

	void ShutdownEngine()
	{
		// Shutdown subsystems
		ShutdownPlatform();
		ShutdownLogger();
		ShutdownMemory();
	}
}