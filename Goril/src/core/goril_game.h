#pragma once
#include "defines.h"


typedef struct GameConfig
{
	u64 game_instance_memory_requirement;
	const char* windowTitle;
	u32 windowWidth;
	u32 windowHeight;
} GameConfig;

typedef bool (*PFN_Init)();
typedef bool (*PFN_Update)();
typedef bool (*PFN_Shutdown)();

typedef struct GameFunctions
{
	PFN_Init     GameInit;
	PFN_Update   GameUpdate;
	PFN_Shutdown GameShutdown;
} GameFunctions;
