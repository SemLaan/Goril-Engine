#pragma once
#include "core/goril_game.h"
#include "core/application.h"
#include "core/logger.h"
#include "core/asserts.h"
#include "core/meminc.h"

extern void GetGameConfigAndFunctions(GameConfig* out_config, GameFunctions* out_gameFunctions);

int main()
{
	// Engine initialization
	GameFunctions gameFunctions = {};
	{
		GameConfig config = {};
		GetGameConfigAndFunctions(&config, &gameFunctions);

		if (!InitializeEngine(config))
		{
			GRFATAL("Failed to initialize engine, shutting down");
			ShutdownEngine();
			return 1;
		}
	}

	// Running the engine and game
	if (!RunEngine(&gameFunctions))
	{
		GRFATAL("Engine failed while running, shutting down");
		ShutdownEngine();
		return 1;
	}

	ShutdownEngine();
	
	return 0;
}