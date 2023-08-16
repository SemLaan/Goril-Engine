#pragma once
#include "defines.h"
#include "vulkan_types.h"
#include "containers/darray.h"

namespace GR
{
	b8 CreateVulkanInstance(RendererState* state, const Darray<const void*>& requiredExtensions, const Darray<const void*>& requiredLayers);

	void DestroyVulkanInstance(RendererState* state);
}