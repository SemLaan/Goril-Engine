#pragma once
#include "core/goril_game.h"
#include "core/application.h"
#include "core/logger.h"
#include "core/asserts.h"

extern GR::GameConfig GetGameConfig();
extern b8 CreateGameInstance(GR::Blk& out_game);

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
	GR::Blk gameInstanceBlock = GR::Blk();
	if (!CreateGameInstance(gameInstanceBlock))
	{
		GRFATAL("Failed to create game instance, shutting down");
	}
	GRASSERT_DEBUG(gameInstanceBlock.ptr);

	// Running the engine and game
	if (!GR::RunEngine((GR::GorilGame*)gameInstanceBlock.ptr))
	{
		GRFATAL("Engine failed while running, shutting down");
		return 1;
	}

	GR::GetGlobalAllocator()->Free(gameInstanceBlock);

	GR::ShutdownEngine();
	
	return 0;
}