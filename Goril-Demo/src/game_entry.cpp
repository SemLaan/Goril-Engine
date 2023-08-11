#include <goril.h>

#include "game.h"
#include <entrypoint.h>

GR::GameConfig GetGameConfig()
{
	GR::GameConfig config = {};
	
	config.game_instance_memory_requirement = sizeof(Game) + 16;
	config.width = 1280;
	config.height = 720;

	return config;
}

b8 CreateGameInstance(GR::GorilGame*& out_game)
{
	out_game = (GR::GorilGame*)GR::GetGlobalAllocator()->Alloc(sizeof(Game), GR::mem_tag::GAME);
	out_game = new(out_game) Game();

	return true;
}