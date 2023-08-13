#include "game.h"
#include <core/gr_memory.h>
#include <core/event.h>
#include <core/logger.h>

using namespace GR;

b8 Beef(EventType type, EventData data)
{
    GRDEBUG("Test, {}, {}, {}, {}", data.u32[0], data.u32[1], data.u32[2], data.u32[3]);
    return false;
}

b8 Game::Init()
{
    PrintMemoryStats();
    RegisterEventListener(EVCODE_QUIT, Beef);
    EventData data;
    data.u32[0] = 3;
    data.u32[1] = 4;
    data.u32[2] = 5;
    data.u32[3] = 6;
    InvokeEvent(EVCODE_QUIT, data);
    UnregisterEventListener(EVCODE_QUIT, Beef);
    return true;
}

b8 Game::Update()
{
    EventData data;
    data.u32[0] = 3;
    data.u32[1] = 4;
    data.u32[2] = 5;
    data.u32[3] = 6;
    InvokeEvent(EVCODE_QUIT, data);
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
