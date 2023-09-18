#include "vulkan_shader_loader.h"



b8 ReadFile(const char* filename, mem_tag tag, Darray<char>* out_data)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		GRERROR("Failed to open file");
		return false;
	}

	size_t fileSize = (size_t)file.tellg();
	*out_data = CreateDarrayWithSize<char>(tag, (u32)fileSize);

	file.seekg(0);
	file.read(out_data->GetRawElements(), fileSize);

	file.close();

	return true;
}

b8 CreateShaderModule(RendererState* state, Darray<char>& code, VkShaderModule* out_shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.Size();
	// This cast from char* to u32* is possible because darray's are aligned on 64B
	createInfo.pCode = (u32*)code.GetRawElements();

	if (VK_SUCCESS != vkCreateShaderModule(state->device, &createInfo, state->allocator, out_shaderModule))
	{
		GRERROR("Shader module creation failed");
		return false;
	}

	return true;
}

