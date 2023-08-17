#pragma once
#include <vulkan/vulkan.h>
#include "defines.h"
#include "containers/darray.h"

namespace GR
{
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		Darray<VkSurfaceFormatKHR> formats;
		Darray<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		u32 graphicsFamily;
		u32 presentFamily;
	};

	struct RendererState
	{
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		QueueFamilyIndices queueIndices;
		VkSurfaceKHR surface;
		SwapchainSupportDetails swapchainSupport;
		VkSwapchainKHR swapchain;
		Darray<VkImage> swapchainImages;
		Darray<VkImageView> swapchainImageViews;
		VkFormat swapchainFormat;
		VkExtent2D swapchainExtent;
		VkRenderPass renderpass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		Darray<VkFramebuffer> swapchainFramebuffers;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;
		VkAllocationCallbacks* allocator;
#ifndef GR_DIST
		VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
	};
}