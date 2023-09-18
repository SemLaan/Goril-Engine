#pragma once
#include "defines.h"
#include "vulkan_types.h"




b8 CreateQueues();
void DestroyQueues();

void TryDestroyResourcesPendingDestruction();
