#pragma once
#include "defines.h"
#include "vulkan_types.h"


struct VulkanCreateImageParameters
{
	u32						width;
	u32						height;
	VkFormat				format;
	VkImageTiling			tiling;
	VkImageUsageFlags		usage;
	VkMemoryPropertyFlags	properties;
};

bool CreateImage(VulkanCreateImageParameters* pCreateParameters, VkImage* pImage, VkDeviceMemory* pMemory);

///TODO: create image view helper function maybe? since swapchain also needs it
