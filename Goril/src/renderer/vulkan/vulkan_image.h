#pragma once
#include "defines.h"
#include "vulkan_types.h"

namespace GR
{
	struct VulkanCreateImageParameters
	{
		u32						width;
		u32						height;
		VkFormat				format;
		VkImageTiling			tiling;
		VkImageUsageFlags		usage;
		VkMemoryPropertyFlags	properties;
	};

	b8 CreateImage(VulkanCreateImageParameters* pCreateParameters, VkImage* pImage, VkDeviceMemory* pMemory);

	///TODO: create image view helper function maybe? since swapchain also needs it
}