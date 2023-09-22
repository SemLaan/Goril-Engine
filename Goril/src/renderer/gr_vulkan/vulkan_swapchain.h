#pragma once
#include "defines.h"
#include "vulkan_types.h"


bool CreateSwapchain(RendererState* state);

void DestroySwapchain(RendererState* state);

bool CreateSwapchainFramebuffers(RendererState* state);

void DestroySwapchainFramebuffers(RendererState* state);
