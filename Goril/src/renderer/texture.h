#pragma once
#include "defines.h"
#include "renderer_types.h"


namespace GR
{
	Texture CreateTexture(u32 width, u32 height, void* pixels);
	void DestroyTexture(Texture clientTexture);
}