#pragma once
#include "core/goril_game.h"
#include "core/application.h"
#include "core/logger.h"
#include "core/asserts.h"

extern gr::game_config GetGameConfig();
extern b8 CreateGameInstance(gr::GorilGame* out_game);

int main()
{
	// Engine initialization
	{
		gr::game_config config = GetGameConfig();

		if (!gr::initialize_engine(config))
		{
			GRFATAL("Failed to initialize engine, shutting down");
			return 1;
		}
	}
	
	// Getting the game instance object
	gr::GorilGame* game_instance = (gr::GorilGame*)malloc(4000); // TODO: use engine allocator
	if (!CreateGameInstance(game_instance))
	{
		GRFATAL("Failed to create game instance, shutting down");
	}
	GRASSERT_DEBUG(game_instance);

	// Running the engine and game
	if (!gr::run_engine(game_instance))
	{
		GRFATAL("Engine failed while running, shutting down");
		return 1;
	}
	
	return 0;
}