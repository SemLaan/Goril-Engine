#pragma once
#include "core/goril_game.h"
#include "core/application.h"
#include "core/logger.h"
#include "core/asserts.h"
#include "core/gr_memory.h"

extern GameConfig GetGameConfig();
extern b8 CreateGameInstance(GorilGame*& out_game);

int main()
{
	// Engine initialization
	{
		GameConfig config = GetGameConfig();

		if (!InitializeEngine(config))
		{
			GRFATAL("Failed to initialize engine, shutting down");
			ShutdownEngine();
			GRASSERT_DEBUG(false);
			return 1;
		}
	}
	
	// Getting the game instance object
	GorilGame* gameInstance = nullptr;
	if (!CreateGameInstance(gameInstance))
	{
		GRFATAL("Failed to create game instance, shutting down");
		ShutdownEngine();
		GRASSERT_DEBUG(false);
		return 1;
	}
	GRASSERT_DEBUG(gameInstance);

	// Running the engine and game
	if (!RunEngine(gameInstance))
	{
		GRFATAL("Engine failed while running, shutting down");
		GRFree(gameInstance);
		ShutdownEngine();
		GRASSERT_DEBUG(false);
		return 1;
	}

	GRFree(gameInstance);

	ShutdownEngine();
	
	return 0;
}