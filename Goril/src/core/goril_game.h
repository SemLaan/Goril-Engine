#pragma once
#include "defines.h"

namespace GR
{
	struct GameConfig
	{
		size_t game_instance_memory_requirement;
		u32 width;
		u32 height;
	};

	class GRAPI GorilGame
	{
	public:
		virtual b8 Init() = 0;

		virtual b8 Update() = 0;

		virtual b8 Render() = 0;

		virtual b8 Shutdown() = 0;
	};
}