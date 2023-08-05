#pragma once
#include "defines.h"

namespace gr
{
	struct game_config
	{
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