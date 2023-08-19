#include "vulkan_swapchain.h"
#include "core/platform.h"
#include "vulkan_device.h"

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

		// Setting swapchain resolution to window size
		u32 windowWidth, windowHeight;
		GetPlatformWindowSize(&windowWidth, &windowHeight);
		VkExtent2D swapchainExtent = { windowWidth, windowHeight };
		// Making sure the swapchain isn't too big or too small
		state->swapchainSupport.formats.Deinitialize();
		state->swapchainSupport.presentModes.Deinitialize();
		state->swapchainSupport = QuerySwapchainSupport(state->physicalDevice, state->surface);
		if (swapchainExtent.width > state->swapchainSupport.capabilities.maxImageExtent.width)
			swapchainExtent.width = state->swapchainSupport.capabilities.maxImageExtent.width;
		if (swapchainExtent.height > state->swapchainSupport.capabilities.maxImageExtent.height)
			swapchainExtent.height = state->swapchainSupport.capabilities.maxImageExtent.height;
		if (swapchainExtent.width < state->swapchainSupport.capabilities.minImageExtent.width)
			swapchainExtent.width = state->swapchainSupport.capabilities.minImageExtent.width;
		if (swapchainExtent.height < state->swapchainSupport.capabilities.minImageExtent.height)
			swapchainExtent.height = state->swapchainSupport.capabilities.minImageExtent.height;

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

		state->swapchainFormat = format.format;
		state->swapchainExtent = swapchainExtent;

		u32 swapchainImageCount = 0;
		vkGetSwapchainImagesKHR(state->device, state->swapchain, &swapchainImageCount, nullptr);
		state->swapchainImages = CreateDarrayWithSize<VkImage>(MEM_TAG_RENDERER_SUBSYS, swapchainImageCount);
		vkGetSwapchainImagesKHR(state->device, state->swapchain, &swapchainImageCount, state->swapchainImages.GetRawElements());

		state->swapchainImageViews = CreateDarrayWithSize<VkImageView>(MEM_TAG_RENDERER_SUBSYS, swapchainImageCount);

		for (u32 i = 0; i < swapchainImageCount; ++i)
		{
			VkImageViewCreateInfo viewCreateInfo = {};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.pNext = nullptr;
			viewCreateInfo.flags = 0;
			viewCreateInfo.image = state->swapchainImages[i];
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = state->swapchainFormat;
			viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;

			if (VK_SUCCESS != vkCreateImageView(state->device, &viewCreateInfo, state->allocator, &state->swapchainImageViews[i]))
			{
				GRFATAL("Swapchain image view creation failed");
				return false;
			}
		}

		return true;
	}

	void DestroySwapchain(RendererState* state)
	{
		if (state->swapchainImageViews.GetRawElements())
		{
			for (u32 i = 0; i < state->swapchainImageViews.Size(); ++i)
			{
				vkDestroyImageView(state->device, state->swapchainImageViews[i], state->allocator);
			}
		}

		if (state->swapchain)
			vkDestroySwapchainKHR(state->device, state->swapchain, state->allocator);

		if (state->swapchainImages.GetRawElements())
			state->swapchainImages.Deinitialize();
		if (state->swapchainImageViews.GetRawElements())
			state->swapchainImageViews.Deinitialize();
	}

	b8 CreateSwapchainFramebuffers(RendererState* state)
	{
		state->swapchainFramebuffers = CreateDarrayWithSize<VkFramebuffer>(MEM_TAG_RENDERER_SUBSYS, (u32)state->swapchainImages.Size());

		for (u32 i = 0; i < state->swapchainFramebuffers.Size(); ++i)
		{
			VkImageView attachments[] = { state->swapchainImageViews[i] };

			VkFramebufferCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.renderPass = state->renderpass;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = attachments;
			createInfo.width = state->swapchainExtent.width;
			createInfo.height = state->swapchainExtent.height;
			createInfo.layers = 1;

			if (VK_SUCCESS != vkCreateFramebuffer(state->device, &createInfo, state->allocator, &state->swapchainFramebuffers[i]))
			{
				GRFATAL("Swapchain framebuffer creation failed");
				return false;
			}
		}

		return true;
	}

	void DestroySwapchainFramebuffers(RendererState* state)
	{
		if (state->swapchainFramebuffers.GetRawElements())
		{
			for (u32 i = 0; i < state->swapchainFramebuffers.Size(); ++i)
			{
				vkDestroyFramebuffer(state->device, state->swapchainFramebuffers[i], state->allocator);
			}
			state->swapchainFramebuffers.Deinitialize();
		}
	}
}