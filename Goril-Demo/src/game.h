#pragma once
#include <core/goril_game.h>

class Game : public GR::GorilGame
{
public:
	// Inherited via GorilGame
	b8 Init() override;
	b8 Update() override;
	b8 Render() override;
	b8 Shutdown() override;
};