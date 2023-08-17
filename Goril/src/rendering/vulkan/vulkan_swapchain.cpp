#include "vulkan_swapchain.h"


namespace GR
{

	b8 CreateSwapchain(RendererState* state)
	{
		// Getting a swapchain format
		VkSurfaceFormatKHR format = state->swapchainSupport.formats[0];
		for (u32 i = 0; i < state->swapchainSupport.formats.Size(); ++i)
		{
			VkSurfaceFormatKHR availableFormat = state->swapchainSupport.formats[i];
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				format = availableFormat;
		}

		// Getting a presentation mode
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (u32 i = 0; i < state->swapchainSupport.presentModes.Size(); ++i)
		{
			VkPresentModeKHR availablePresentMode = state->swapchainSupport.presentModes[i];
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = availablePresentMode;
		}

		// Setting swapchain resolution
		VkExtent2D swapchainExtent = state->swapchainSupport.capabilities.currentExtent;

		u32 imageCount = state->swapchainSupport.capabilities.minImageCount + 1;
		if (imageCount > state->swapchainSupport.capabilities.maxImageCount)
			imageCount = state->swapchainSupport.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = state->surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = format.format;
		createInfo.imageColorSpace = format.colorSpace;
		createInfo.imageExtent = swapchainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		u32 queueFamilyIndices[2] = { state->queueIndices.graphicsFamily, state->queueIndices.presentFamily };
		if (state->queueIndices.graphicsFamily != state->queueIndices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		
		createInfo.preTransform = state->swapchainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (VK_SUCCESS != vkCreateSwapchainKHR(state->device, &createInfo, state->allocator, &state->swapchain))
		{
			GRFATAL("Vulkan Swapchain creation failed");
			return false;
		}

		return true;
	}

	void DestroySwapchain(RendererState* state)
	{
		if (state->swapchain)
			vkDestroySwapchainKHR(state->device, state->swapchain, state->allocator);
	}
}