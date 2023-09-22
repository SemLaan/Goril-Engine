#pragma once
#include "defines.h"
#include "renderer_types.h"


#define TEXTURE_CHANNELS 4


Texture TextureCreate(u32 width, u32 height, void* pixels);
void TextureDestroy(Texture clientTexture);
