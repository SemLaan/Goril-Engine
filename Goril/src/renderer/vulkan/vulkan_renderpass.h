#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

b8 CreateRenderpass(RendererState* state);

void DestroyRenderpass(RendererState* state);
