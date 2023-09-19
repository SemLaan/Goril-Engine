#pragma once
#include "defines.h"
#include <fstream>
#include "containers/darray.h"
#include "vulkan_types.h"


bool ReadFile(const char* filename, mem_tag tag, char** out_data);

bool CreateShaderModule(RendererState* state, char* codeDarray, VkShaderModule* out_shaderModule);
