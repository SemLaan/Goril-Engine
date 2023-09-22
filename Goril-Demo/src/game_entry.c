#include "game.h"
#include <entrypoint.h>

GameConfig GetGameConfig()
{
	GameConfig config = {};
	
	config.game_instance_memory_requirement = KiB;
	config.windowTitle = "Test game";
	config.width = 1280;
	config.height = 720;

	return config;
}

void GetGameConfigAndFunctions(GameConfig* out_config, GameFunctions* out_gameFunctions)
{
	out_config->game_instance_memory_requirement = KiB;
	out_config->windowTitle = "Test game";
	out_config->width = 1280;
	out_config->height = 720;

	out_gameFunctions->GameInit = Init;
	out_gameFunctions->GameUpdate = Update;
	out_gameFunctions->GameRender = Render;
	out_gameFunctions->GameShutdown = Shutdown;
}
