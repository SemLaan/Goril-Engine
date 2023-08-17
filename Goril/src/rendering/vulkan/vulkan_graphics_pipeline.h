#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

namespace GR
{

	b8 CreateGraphicsPipeline(RendererState* state);

	void DestroyGraphicsPipeline(RendererState* state);
}