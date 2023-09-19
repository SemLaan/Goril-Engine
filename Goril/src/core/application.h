#pragma once
#include "defines.h"
#include "goril_game.h"


GRAPI bool InitializeEngine(GameConfig config);

GRAPI bool RunEngine(GameFunctions* gameFunctions);

GRAPI void ShutdownEngine();


