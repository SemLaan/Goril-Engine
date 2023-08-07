#pragma once
#include "defines.h"
#include "goril_game.h"

namespace gr
{

	GRAPI b8 initialize_engine(game_config config);

	GRAPI b8 run_engine(GorilGame* game_instance);
}

