#include "renderer.h"

#include "core/logger.h"
#include "containers/darray.h"
#include "core/gr_memory.h"
#include <vulkan/vulkan.h>

namespace GR
{

	struct RendererState
	{
		VkInstance instance;
		VkAllocationCallbacks* allocator;
	};

	static RendererState* state = nullptr;


	b8 InitializeRenderer()
	{
		GRASSERT_DEBUG(state == nullptr); // If this triggers init got called twice
		GRINFO("Initializing renderer subsystem...");

		state = (RendererState*)GAlloc(sizeof(RendererState), MEM_TAG_RENDERER_SUBSYS);
		state->allocator = nullptr;

		// ================ App info =============================================
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Test app";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.pEngineName = "Goril";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// ================== Getting extensions ================================
		// Getting required extensions
		Darray<const void*> requiredExtensions = Darray<const void*>();
		requiredExtensions.Initialize(MEM_TAG_RENDERER_SUBSYS);
		requiredExtensions.Pushback(&VK_KHR_SURFACE_EXTENSION_NAME);
		requiredExtensions.Pushback(&"VK_KHR_win32_surface");

		// Checking if required extensions are available
		u32 availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		Darray<VkExtensionProperties> availableExtensions = Darray<VkExtensionProperties>();
		availableExtensions.Initialize(MEM_TAG_RENDERER_SUBSYS, availableExtensionCount);
		availableExtensions.Size() = availableExtensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.GetRawElements());
		
		u32 availableRequiredExtensions = 0;
		for (u32 i = 0; i < requiredExtensions.Size(); ++i)
		{
			for (u32 j = 0; j < availableExtensionCount; ++j)
			{
				if (0 == strncmp((const char*)requiredExtensions[i], availableExtensions[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE))
				{
					availableRequiredExtensions++;
				}
			}
		}

		if (availableRequiredExtensions < requiredExtensions.Size())
		{
			GRFATAL("Couldn't find required Vulkan extensions");
			requiredExtensions.Deinitialize();
			availableExtensions.Deinitialize();
			GFree(state);
			state = nullptr;
			return false;
		}
		else
			GRTRACE("Required Vulkan extensions found");

		// ================ Getting layers ===============================


		// ================== Creating instance =================================
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.enabledExtensionCount = (u32)requiredExtensions.Size();
		createInfo.ppEnabledExtensionNames = (const char* const*)requiredExtensions.GetRawElements();

		VkResult result = vkCreateInstance(&createInfo, state->allocator, &state->instance);

		requiredExtensions.Deinitialize();
		availableExtensions.Deinitialize();

		if (result != VK_SUCCESS)
		{
			GRFATAL("Failed to create Vulkan instance");
			GFree(state);
			state = nullptr;
			return false;
		}

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

		vkDestroyInstance(state->instance, nullptr);
		GFree(state);
	}
}