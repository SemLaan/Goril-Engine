#pragma once
#include "defines.h"
#include "goril_game.h"


bool InitializeEngine(GameConfig config);

bool RunEngine(GameFunctions* gameFunctions);

void ShutdownEngine();


