#include "game.h"
#include <entrypoint.h>

GameConfig GetGameConfig()
{
	GameConfig config{};
	
	config.game_instance_memory_requirement = KiB;
	config.windowTitle = L"Test game";
	config.width = 1280;
	config.height = 720;

	return config;
}

bool CreateGameInstance(GorilGame*& out_game)
{
	out_game = (GorilGame*)Alloc(GetGlobalAllocator(), sizeof(Game), MEM_TAG_GAME);
	out_game = new(out_game) Game();

	return true;
}