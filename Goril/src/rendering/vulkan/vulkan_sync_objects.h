#pragma once
#include "defines.h"
#include "vulkan_types.h"

namespace GR
{
	b8 CreateSyncObjects(RendererState* state);

	void DestroySyncObjects(RendererState* state);
}