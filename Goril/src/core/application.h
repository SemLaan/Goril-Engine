#pragma once
#include "defines.h"
#include "goril_game.h"


GRAPI bool InitializeEngine(GameConfig config);

GRAPI bool RunEngine(GorilGame* gameInstance);

GRAPI void ShutdownEngine();


