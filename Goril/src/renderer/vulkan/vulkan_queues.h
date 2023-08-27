#pragma once
#include "defines.h"
#include "vulkan_types.h"


namespace GR
{

	b8 CreateQueues();
	void DestroyQueues();

	void TryDestroyResourcesPendingDestruction();
}