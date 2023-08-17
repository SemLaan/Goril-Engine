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

		// ======================== Creating graphics pipeline ============================================
		if (!CreateGraphicsPipeline(state))
			return false;

		return true;
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

		// ====================== Destroying graphics pipeline if it was created ================================
		DestroyGraphicsPipeline(state);

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