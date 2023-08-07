#include <goril.h>

#include "game.h"
#include <entrypoint.h>

gr::game_config GetGameConfig()
{
	gr::game_config config = {};
	
	config.game_instance_memory_requirement = sizeof(Game);
	config.width = 1280;
	config.height = 720;

	return config;
}

b8 CreateGameInstance(gr::GorilGame* out_game)
{
	out_game = new(out_game) Game();

	return true;
}