#pragma once
#include <goril.h>

class Game : public gr::GorilGame
{
	// Inherited via GorilGame
	b8 Init() override;
	b8 Update() override;
	b8 Render() override;
	b8 Shutdown() override;
};