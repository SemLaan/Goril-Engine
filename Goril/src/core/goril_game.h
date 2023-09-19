#pragma once
#include "defines.h"


struct GameConfig
{
	size_t game_instance_memory_requirement;
	const wchar_t* windowTitle;
	u32 width;
	u32 height;
};

class GRAPI GorilGame
{
public:
	virtual bool Init() = 0;

	virtual bool Update() = 0;

	virtual bool Render() = 0;

	virtual bool Shutdown() = 0;
};
