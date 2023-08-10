#include <goril.h>

#include "game.h"
#include <entrypoint.h>

GR::GameConfig GetGameConfig()
{
	GR::GameConfig config = {};
	
	config.game_instance_memory_requirement = sizeof(Game);
	config.width = 1280;
	config.height = 720;

	return config;
}

b8 CreateGameInstance(GR::Blk& out_game)
{
	out_game = GR::GetGlobalAllocator()->Alloc(sizeof(Game), GR::mem_tag::GAME);
	out_game.ptr = new(out_game.ptr) Game();

	return true;
}