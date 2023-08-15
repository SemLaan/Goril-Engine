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
	};

	static RendererState* state = nullptr;


	b8 InitializeRenderer()
	{
		GRASSERT_DEBUG(state == nullptr); // If this triggers init got called twice
		GRINFO("Initializing renderer subsystem...");

		state = (RendererState*)GAlloc(sizeof(RendererState), MEM_TAG_RENDERER_SUBSYS);

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Test app";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.pEngineName = "Goril";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		Darray<const void*> extensionNames = Darray<const void*>();
		extensionNames.Initialize(MEM_TAG_RENDERER_SUBSYS);
		extensionNames.Pushback(&VK_KHR_SURFACE_EXTENSION_NAME);
		extensionNames.Pushback(&"VK_KHR_win32_surface");

		u32 availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		Darray<VkExtensionProperties> availableExtensions = Darray<VkExtensionProperties>();
		availableExtensions.Initialize(MEM_TAG_RENDERER_SUBSYS, availableExtensionCount);
		availableExtensions.Size() = availableExtensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.GetRawElements());

		for (u32 i = 0; i < availableExtensionCount; ++i)
		{
			GRDEBUG("{}", availableExtensions[i].extensionName);
		}

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.enabledExtensionCount = extensionNames.Size();
		createInfo.ppEnabledExtensionNames = (const char* const*)extensionNames.GetRawElements();

		VkResult result = vkCreateInstance(&createInfo, nullptr, &state->instance);

		extensionNames.Deinitialize();
		availableExtensions.Deinitialize();

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