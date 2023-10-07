#include "game.h"
#include <entrypoint.h>


void GetGameConfigAndFunctions(GameConfig* out_config, GameFunctions* out_gameFunctions)
{
	out_config->game_instance_memory_requirement = KiB;
	out_config->windowTitle = "Test game";
	out_config->windowWidth = 1280;
	out_config->windowHeight = 720;

	out_gameFunctions->GameInit = Init;
	out_gameFunctions->GameUpdate = Update;
	out_gameFunctions->GameRender = Render;
	out_gameFunctions->GameShutdown = Shutdown;
}
