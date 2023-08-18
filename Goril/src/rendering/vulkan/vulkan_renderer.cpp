#include "../renderer.h"

#include "core/logger.h"
#include "core/gr_memory.h"
#include "containers/darray.h"

#include "vulkan_platform.h"
#include "vulkan_types.h"
#include "vulkan_debug_messenger.h"
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_graphics_pipeline.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_pool.h"
#include "vulkan_sync_objects.h"

namespace GR
{

	static RendererState* state = nullptr;

	b8 InitializeRenderer()
	{
		GRASSERT_DEBUG(state == nullptr); // If this triggers init got called twice
		GRINFO("Initializing renderer subsystem...");

		state = (RendererState*)GAlloc(sizeof(RendererState), MEM_TAG_RENDERER_SUBSYS);
		Zero(state, sizeof(RendererState));
		state->allocator = nullptr;


		// ================== Getting required extensions and layers ================================
		// Getting required extensions
		Darray<const void*> requiredExtensions = Darray<const void*>();
		requiredExtensions.Initialize(MEM_TAG_RENDERER_SUBSYS);
		GetPlatformExtensions(&requiredExtensions);
		requiredExtensions.Pushback(&VK_KHR_SURFACE_EXTENSION_NAME);
#ifndef GR_DIST
		requiredExtensions.Pushback(&VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // !GR_DIST

		// Getting required layers
		Darray<const void*> requiredLayers = Darray<const void*>();
		requiredLayers.Initialize(MEM_TAG_RENDERER_SUBSYS);
#ifndef GR_DIST
		requiredLayers.Pushback(&"VK_LAYER_KHRONOS_validation");
#endif // !GR_DIST


		// ================== Creating instance =================================
		if (!CreateVulkanInstance(state, requiredExtensions, requiredLayers))
		{
			requiredExtensions.Deinitialize();
			requiredLayers.Deinitialize();
			return false;
		}
		requiredExtensions.Deinitialize();

		// =============== Creating debug messenger ============================
#ifndef GR_DIST
		if (!CreateDebugMessenger(state))
		{
			requiredLayers.Deinitialize();
			return false;
		}
#endif // !GR_DIST

		// ================ Creating a surface =====================================
		if (!PlatformCreateSurface(state->instance, state->allocator, &state->surface))
		{
			GRFATAL("Failed to create Vulkan surface");
			requiredLayers.Deinitialize();
			return false;
		}

		// ================ Getting a physical device ==============================
		Darray<const void*> requiredDeviceExtensions = Darray<const void*>();
		requiredDeviceExtensions.Initialize(MEM_TAG_RENDERER_SUBSYS);
		requiredDeviceExtensions.Pushback(&VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		
		if (!SelectPhysicalDevice(state, &requiredDeviceExtensions))
		{
			requiredDeviceExtensions.Deinitialize();
			requiredLayers.Deinitialize();
			return false;
		}

		// ================== Getting device queue families ==============================
		SelectQueueFamilies(state);

		// ===================== Creating logical device =============================================
		if (!CreateLogicalDevice(state, &requiredDeviceExtensions, &requiredLayers))
		{
			requiredLayers.Deinitialize();
			requiredDeviceExtensions.Deinitialize();
			return false;
		}
		requiredLayers.Deinitialize();
		requiredDeviceExtensions.Deinitialize();

		// ======================== Creating the swapchain ===============================================
		if (!CreateSwapchain(state))
			return false;

		// ========================== Creating renderpass ==============================================
		if (!CreateRenderpass(state))
			return false;

		// ======================== Creating graphics pipeline ============================================
		if (!CreateGraphicsPipeline(state))
			return false;

		// ======================= Create swapchain framebuffers ============================================
		if (!CreateSwapchainFramebuffers(state))
			return false;

		// ========================== Create command pool =======================================
		if (!CreateCommandPool(state))
			return false;

		// ============================ Allocate a command buffer =======================================
		if (!AllocateCommandBuffer(state))
			return false;

		// ================================ Create sync objects ===========================================
		if (!CreateSyncObjects(state))
			return false;

		return true;
	}

	void UpdateRenderer()
	{
		vkWaitForFences(state->device, 1, &state->inFlightFence, VK_TRUE, UINT64_MAX);

		vkResetFences(state->device, 1, &state->inFlightFence);

		u32 imageIndex;
		vkAcquireNextImageKHR(state->device, state->swapchain, UINT64_MAX, state->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		vkResetCommandBuffer(state->commandBuffer, 0);

		RecordCommandBuffer(state, imageIndex);

		VkSemaphore waitSemaphores[] = { state->imageAvailableSemaphore };
		VkSemaphore signalSemaphores[] = { state->renderFinishedSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &state->commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (VK_SUCCESS != vkQueueSubmit(state->graphicsQueue, 1, &submitInfo, state->inFlightFence))
		{
			GRERROR("Failed to submit queue");
			return;
		}

		VkSwapchainKHR swapchains[] = { state->swapchain };

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(state->graphicsQueue, &presentInfo);
	}

	void ShutdownRenderer()
	{
		if (state == nullptr)
		{
			GRINFO("Renderer startup failed, skipping shutdown");
			return;
		}
		else
		{
			GRINFO("Shutting down renderer subsystem...");
		}

		vkDeviceWaitIdle(state->device);

		// ================================ Destroy sync objects if they were created ===========================================
		DestroySyncObjects(state);

		// ======================== Destroying command pool if it was created ====================================
		DestroyCommandPool(state);

		// ====================== Destroying swapchain framebuffers if they were created ================================
		DestroySwapchainFramebuffers(state);

		// ====================== Destroying graphics pipeline if it was created ================================
		DestroyGraphicsPipeline(state);

		// ======================== Destroying renderpass if it was created =====================================
		DestroyRenderpass(state);

		// ====================== Destroying swapchain if it was created ================================
		DestroySwapchain(state);

		// ===================== Destroying logical device if it was created =================================
		DestroyLogicalDevice(state);

		// ======================= Destroying the surface if it was created ==================================
		if (state->surface)
			vkDestroySurfaceKHR(state->instance, state->surface, state->allocator);

		// ===================== Destroying debug messenger if it was created =================================
#ifndef GR_DIST
		DestroyDebugMessenger(state);
#endif // !GR_DIST
		
		// ======================= Destroying instance if it was created =======================================
		DestroyVulkanInstance(state);
		GFree(state);
		state = nullptr;
	}
}