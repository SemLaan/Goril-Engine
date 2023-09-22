#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

bool CreateRenderpass(RendererState* state);

void DestroyRenderpass(RendererState* state);
