#include "game.h"
#include <core/gr_memory.h>
#include <core/event.h>
#include <core/logger.h>

using namespace GR;

b8 Beef(EventType type, EventData data)
{
    GRDEBUG("Test, {}, {}, {}, {}", data.uint32[0], data.uint32[1], data.uint32[2], data.uint32[3]);
    return false;
}

b8 Game::Init()
{
    PrintMemoryStats();
    RegisterEventListener(EVCODE_QUIT, Beef);
    return true;
}

b8 Game::Update()
{
    InvokeEvent(EVCODE_QUIT, EventData({3,4,5,6}));
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
