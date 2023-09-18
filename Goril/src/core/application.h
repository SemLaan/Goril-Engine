#pragma once
#include "defines.h"
#include "goril_game.h"


GRAPI b8 InitializeEngine(GameConfig config);

GRAPI b8 RunEngine(GorilGame* gameInstance);

GRAPI void ShutdownEngine();


