#include "game.h"
#include <core/gr_memory.h>
#include <core/logger.h>
//#include <core/input.h>
//#include <core/event.h>

using namespace GR;

b8 Game::Init()
{
    PrintMemoryStats();
    return true;
}

b8 Game::Update()
{
    return true;
}

b8 Game::Render()
{
    return true;
}

b8 Game::Shutdown()
{
    return true;
}
