#pragma once
#include "defines.h"
#include "renderer_types.h"


#define TEXTURE_CHANNELS 4




Texture GRAPI CreateTexture(u32 width, u32 height, void* pixels);
void GRAPI DestroyTexture(Texture clientTexture);
