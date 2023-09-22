#pragma once
#include "defines.h"
#include "renderer_types.h"


#define TEXTURE_CHANNELS 4




Texture CreateTexture(u32 width, u32 height, void* pixels);
void DestroyTexture(Texture clientTexture);
