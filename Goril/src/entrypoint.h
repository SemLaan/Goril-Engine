#pragma once
#include "core/goril_game.h"
#include "core/application.h"
#include "core/logger.h"
#include "core/asserts.h"

extern GR::GameConfig GetGameConfig();
extern b8 CreateGameInstance(GR::GorilGame* out_game);

int main()
{
	// Engine initialization
	{
		GR::GameConfig config = GetGameConfig();

		if (!GR::InitializeEngine(config))
		{
			GRFATAL("Failed to initialize engine, shutting down");
			return 1;
		}
	}
	
	// Getting the game instance object
	GR::GorilGame* gameInstance = (GR::GorilGame*)malloc(4000); // TODO: use engine allocator
	if (!CreateGameInstance(gameInstance))
	{
		GRFATAL("Failed to create game instance, shutting down");
	}
	GRASSERT_DEBUG(gameInstance);

	// Running the engine and game
	if (!GR::RunEngine(gameInstance))
	{
		GRFATAL("Engine failed while running, shutting down");
		return 1;
	}

	GR::ShutdownEngine();
	
	return 0;
}