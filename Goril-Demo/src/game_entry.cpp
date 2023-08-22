#include "game.h"
#include <entrypoint.h>

GR::GameConfig GetGameConfig()
{
	GR::GameConfig config{};
	
	config.game_instance_memory_requirement = KiB;
	config.windowTitle = L"Test game";
	config.width = 1280;
	config.height = 720;

	return config;
}

b8 CreateGameInstance(GR::GorilGame*& out_game)
{
	out_game = (GR::GorilGame*)GR::GRAlloc(sizeof(Game), GR::MEM_TAG_GAME);
	out_game = new(out_game) Game();

	return true;
}