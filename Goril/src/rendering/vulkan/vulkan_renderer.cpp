#include "../renderer.h"

#include "core/logger.h"
#include "core/gr_memory.h"
#include "core/event.h"
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
#include "../buffer.h"

namespace GR
{

	RendererState* vk_state = nullptr;

	b8 OnWindowResize(EventCode type, EventData data);

	b8 InitializeRenderer()
	{
		GRASSERT_DEBUG(vk_state == nullptr); // If this triggers init got called twice
		GRINFO("Initializing renderer subsystem...");

		vk_state = (RendererState*)GRAlloc(sizeof(RendererState), MEM_TAG_RENDERER_SUBSYS);
		Zero(vk_state, sizeof(RendererState));
		vk_state->allocator = nullptr;

		vk_state->maxFramesInFlight = 2;
		vk_state->currentFrame = 0;
		vk_state->shouldRecreateSwapchain = false;

		RegisterEventListener(EVCODE_WINDOW_RESIZED, OnWindowResize);

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
		if (!CreateVulkanInstance(requiredExtensions, requiredLayers))
		{
			requiredExtensions.Deinitialize();
			requiredLayers.Deinitialize();
			return false;
		}
		requiredExtensions.Deinitialize();

		// =============== Creating debug messenger ============================
#ifndef GR_DIST
		if (!CreateDebugMessenger())
		{
			requiredLayers.Deinitialize();
			return false;
		}
#endif // !GR_DIST

		// ================ Creating a surface =====================================
		if (!PlatformCreateSurface(vk_state->instance, vk_state->allocator, &vk_state->surface))
		{
			GRFATAL("Failed to create Vulkan surface");
			requiredLayers.Deinitialize();
			return false;
		}

		// ================ Getting a physical device ==============================
		Darray<const void*> requiredDeviceExtensions = Darray<const void*>();
		requiredDeviceExtensions.Initialize(MEM_TAG_RENDERER_SUBSYS);
		requiredDeviceExtensions.Pushback(&VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		
		if (!SelectPhysicalDevice(vk_state, &requiredDeviceExtensions))
		{
			requiredDeviceExtensions.Deinitialize();
			requiredLayers.Deinitialize();
			return false;
		}

		// ================== Getting device queue families ==============================
		SelectQueueFamilies(vk_state);

		// ===================== Creating logical device =============================================
		if (!CreateLogicalDevice(vk_state, &requiredDeviceExtensions, &requiredLayers))
		{
			requiredLayers.Deinitialize();
			requiredDeviceExtensions.Deinitialize();
			return false;
		}
		requiredLayers.Deinitialize();
		requiredDeviceExtensions.Deinitialize();

		// ======================== Creating the swapchain ===============================================
		if (!CreateSwapchain(vk_state))
			return false;

		// ========================== Creating renderpass ==============================================
		if (!CreateRenderpass(vk_state))
			return false;

		// ======================== Creating graphics pipeline ============================================
		if (!CreateGraphicsPipeline(vk_state))
			return false;

		// ======================= Create swapchain framebuffers ============================================
		if (!CreateSwapchainFramebuffers(vk_state))
			return false;

		// ========================== Create command pool =======================================
		if (!CreateCommandPool())
			return false;

		// ============================ Allocate a command buffer =======================================
		if (!AllocateCommandBuffers())
			return false;

		// ================================ Create sync objects ===========================================
		if (!CreateSyncObjects(vk_state))
			return false;

		/// TODO: put this in application code
		Darray<Vertex> vertices = Darray<Vertex>();
		vertices.Initialize(MEM_TAG_RENDERER_SUBSYS);
		vertices.Pushback({ {-0.5f, -0.5f}, {1.f, 0.f, 0.f} });
		vertices.Pushback({ {0.5f, -0.5f}, {0.f, 1.f, 0.f} });
		vertices.Pushback({ {0.5f, 0.5f}, {0.f, 0.f, 1.f} });
		vertices.Pushback({ {-0.5f, 0.5f}, {1.f, 1.f, 1.f} });
		vk_state->vertexBuffer = CreateVertexBuffer(vertices.GetRawElements(), sizeof(Vertex) * vertices.Size());
		vertices.Deinitialize();

		return true;
	}

	b8 UpdateRenderer()
	{
		if (vk_state->shouldRecreateSwapchain)
			RecreateSwapchain();

		vkWaitForFences(vk_state->device, 1, &vk_state->inFlightFences[vk_state->currentFrame], VK_TRUE, UINT64_MAX);

		u32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(vk_state->device, vk_state->swapchain, UINT64_MAX, vk_state->imageAvailableSemaphores[vk_state->currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			vk_state->shouldRecreateSwapchain = true;
			return false;
		}
		else if (result == VK_SUBOPTIMAL_KHR)
		{
			// Sets recreate swapchain to true BUT DOES NOT RETURN because the image has been acquired so we can continue rendering for this frame
			vk_state->shouldRecreateSwapchain = true;
		}
		else if (result != VK_SUCCESS)
		{
			GRWARN("Failed to acquire next swapchain image");
			return false;
		}

		vkResetFences(vk_state->device, 1, &vk_state->inFlightFences[vk_state->currentFrame]);

		vkResetCommandBuffer(vk_state->commandBuffers[vk_state->currentFrame], 0);

		RecordCommandBuffer(vk_state->commandBuffers[vk_state->currentFrame], imageIndex);

		VkSemaphore waitSemaphores[] = { vk_state->imageAvailableSemaphores[vk_state->currentFrame] };
		VkSemaphore signalSemaphores[] = { vk_state->renderFinishedSemaphores[vk_state->currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vk_state->commandBuffers[vk_state->currentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (VK_SUCCESS != vkQueueSubmit(vk_state->graphicsQueue, 1, &submitInfo, vk_state->inFlightFences[vk_state->currentFrame]))
		{
			GRERROR("Failed to submit queue");
			return false;
		}

		VkSwapchainKHR swapchains[] = { vk_state->swapchain };

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(vk_state->graphicsQueue, &presentInfo);

		vk_state->currentFrame = (vk_state->currentFrame + 1) % vk_state->maxFramesInFlight;

		return true;
	}

	void ShutdownRenderer()
	{
		if (vk_state == nullptr)
		{
			GRINFO("Renderer startup failed, skipping shutdown");
			return;
		}
		else
		{
			GRINFO("Shutting down renderer subsystem...");
		}

		UnregisterEventListener(EVCODE_WINDOW_RESIZED, OnWindowResize);

		vkDeviceWaitIdle(vk_state->device);

		if (vk_state->vertexBuffer)
			DestroyVertexBuffer(vk_state->vertexBuffer);

		// ================================ Destroy sync objects if they were created ===========================================
		DestroySyncObjects(vk_state);

		// ======================== Destroying command pool if it was created ====================================
		DestroyCommandPool();

		// ====================== Destroying swapchain framebuffers if they were created ================================
		DestroySwapchainFramebuffers(vk_state);

		// ====================== Destroying graphics pipeline if it was created ================================
		DestroyGraphicsPipeline(vk_state);

		// ======================== Destroying renderpass if it was created =====================================
		DestroyRenderpass(vk_state);

		// ====================== Destroying swapchain if it was created ================================
		DestroySwapchain(vk_state);

		// ===================== Destroying logical device if it was created =================================
		DestroyLogicalDevice(vk_state);

		// ======================= Destroying the surface if it was created ==================================
		if (vk_state->surface)
			vkDestroySurfaceKHR(vk_state->instance, vk_state->surface, vk_state->allocator);

		// ===================== Destroying debug messenger if it was created =================================
#ifndef GR_DIST
		DestroyDebugMessenger();
#endif // !GR_DIST
		
		// ======================= Destroying instance if it was created =======================================
		DestroyVulkanInstance();
		GRFree(vk_state);
		vk_state = nullptr;
	}

	void RecreateSwapchain()
	{
		vkDeviceWaitIdle(vk_state->device);

		DestroySwapchainFramebuffers(vk_state);
		DestroySwapchain(vk_state);

		CreateSwapchain(vk_state);
		CreateSwapchainFramebuffers(vk_state);

		vk_state->shouldRecreateSwapchain = false;
		GRINFO("Vulkan Swapchain resized");
	}

	b8 OnWindowResize(EventCode type, EventData data)
	{
		vk_state->shouldRecreateSwapchain = true;
		return false;
	}
}