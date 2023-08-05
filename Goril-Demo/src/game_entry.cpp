#include <goril.h>

#include "game.h"
#include <entrypoint.h>

gr::GorilGame* CreateGameInstance()
{
	return new Game();
}