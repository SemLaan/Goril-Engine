#include "application.h"

#include "logger.h"
#include "platform.h"
#include "gr_memory.h"

namespace GR
{
	b8 InitializeEngine(GameConfig config)
	{
		size_t subsystemsRequiredMemory = GetLoggerRequiredMemory() + GetPlatformRequiredMemory();

		// Initialize subsystems
		if (!InitializeMemory(config.game_instance_memory_requirement + subsystemsRequiredMemory))
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

		return true;
	}

	b8 RunEngine(GorilGame* gameInstance)
	{
		while (true)
		{
			// TODO: Do game engine stuff
		}

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