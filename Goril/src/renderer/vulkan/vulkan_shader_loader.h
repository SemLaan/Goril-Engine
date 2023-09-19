#pragma once
#include "defines.h"
#include <fstream>
#include "containers/darray.h"
#include "vulkan_types.h"


b8 ReadFile(const char* filename, mem_tag tag, char** out_data);

b8 CreateShaderModule(RendererState* state, char* codeDarray, VkShaderModule* out_shaderModule);
