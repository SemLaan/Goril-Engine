#pragma once
#include "core/goril_game.h"
#include "core/application.h"
#include "core/logger.h"
#include "core/asserts.h"


extern gr::GorilGame* CreateGameInstance();

int main()
{
	gr::GorilGame* game_instance = CreateGameInstance();
	GRASSERT_DEBUG(game_instance);

	if (!gr::initialize_engine(game_instance))
	{
		GRFATAL("Failed to initialize engine, shutting down");
		return 1;
	}

	if (!gr::run_engine())
	{
		GRFATAL("Engine failed while running, shutting down");
		return 1;
	}
	
	return 0;
}