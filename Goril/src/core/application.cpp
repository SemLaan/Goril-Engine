#include "application.h"

#include "logger.h"
#include "platform.h"

namespace gr
{
	b8 initialize_engine(GorilGame* game_instance)
	{
		// Initialize subsystems
		if (!initialize_logger())
		{
			GRFATAL("Logger failed to initialize");
			return false;
		}
		initialize_platform();

		return true;
	}

	b8 run_engine() 
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