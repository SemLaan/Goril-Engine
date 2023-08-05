#pragma once
#include "core/goril_game.h"
#include "core/application.h"

// TODO: remove this
#include <iostream>

extern gr::GorilGame* CreateGameInstance();

int main()
{
	gr::GorilGame* game_instance = CreateGameInstance();

	gr::initialize_engine(game_instance);

	gr::run_engine();

	std::cin.get();
	return 0;
}