#include "application.h"

#include "logger.h"
#include "platform.h"
#include "memory.h"

namespace GR
{
	b8 InitializeEngine(GameConfig config)
	{
		// Initialize subsystems
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

		// Shutdown subsystems
		ShutdownPlatform();
		ShutdownLogger();

		return true;
	}
}