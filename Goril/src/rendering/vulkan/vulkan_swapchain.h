#pragma once
#include "defines.h"
#include "vulkan_types.h"

namespace GR
{

	b8 CreateSwapchain(RendererState* state);

	void DestroySwapchain(RendererState* state);

	b8 CreateSwapchainFramebuffers(RendererState* state);

	void DestroySwapchainFramebuffers(RendererState* state);
}