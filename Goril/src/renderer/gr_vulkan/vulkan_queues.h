#pragma once
#include "defines.h"
#include "vulkan_types.h"




bool CreateQueues();
void DestroyQueues();

void TryDestroyResourcesPendingDestruction();
