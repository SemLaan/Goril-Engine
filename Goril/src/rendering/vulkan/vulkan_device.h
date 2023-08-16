#pragma once
#include "defines.h"
#include "vulkan_types.h"

namespace GR
{

	b8 SelectPhysicalDevice(RendererState* state);

	b8 CreateLogicalDevice(RendererState* state);

	void DestroyLogicalDevice(RendererState* state);
}