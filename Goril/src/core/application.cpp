#include "application.h"

#include "logger.h"
#include "platform.h"

namespace gr
{
	b8 initialize_engine(game_config config)
	{
		// Initialize subsystems
		if (!initialize_logger())
		{
			GRFATAL("Logger failed to initialize");
			return false;
		}
		if (!initialize_platform())
		{
			GRFATAL("Platform failed to initialize");
			return false;
		}

		return true;
	}

	b8 run_engine(GorilGame* game_instance)
	{
		while (true)
		{
			// TODO: Do game engine stuff
		}

		// Shutdown subsystems
		shutdown_platform();
		shutdown_logger();

		return true;
	}
}