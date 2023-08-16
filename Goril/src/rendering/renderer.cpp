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
		VkPhysicalDevice physicalDevice;
		VkAllocationCallbacks* allocator;
#ifndef GR_DIST
		VkDebugUtilsMessengerEXT debugMessenger;
#endif // !GR_DIST
	};

	static RendererState* state = nullptr;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

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
#ifndef GR_DIST
		requiredExtensions.Pushback(&VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // !GR_DIST

		{
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

			availableExtensions.Deinitialize();

			if (availableRequiredExtensions < requiredExtensions.Size())
			{
				GRFATAL("Couldn't find required Vulkan extensions");
				requiredExtensions.Deinitialize();
				GFree(state);
				state = nullptr;
				return false;
			}
			else
				GRTRACE("Required Vulkan extensions found");
		}

		// ================ Getting layers ===============================
#ifndef GR_DIST
		// Getting required layers
		Darray<const void*> requiredLayers = Darray<const void*>();
		requiredLayers.Initialize(MEM_TAG_RENDERER_SUBSYS);
		requiredLayers.Pushback(&"VK_LAYER_KHRONOS_validation");

		{
			// Checking if required layers are available
			u32 availableLayerCount = 0;
			vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
			Darray<VkLayerProperties> availableLayers = Darray<VkLayerProperties>();
			availableLayers.Initialize(MEM_TAG_RENDERER_SUBSYS, availableLayerCount);
			availableLayers.Size() = availableLayerCount;
			vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.GetRawElements());

			u32 availableRequiredLayers = 0;
			for (u32 i = 0; i < requiredLayers.Size(); ++i)
			{
				for (u32 j = 0; j < availableLayerCount; ++j)
				{
					if (0 == strncmp((const char*)requiredLayers[i], availableLayers[j].layerName, VK_MAX_EXTENSION_NAME_SIZE))
					{
						availableRequiredLayers++;
					}
				}
			}

			availableLayers.Deinitialize();

			if (availableRequiredLayers < requiredLayers.Size())
			{
				GRFATAL("Couldn't find required Vulkan layers");
				requiredExtensions.Deinitialize();
				requiredLayers.Deinitialize();
				GFree(state);
				state = nullptr;
				return false;
			}
			else
				GRTRACE("Required Vulkan layers found");
		}
		
		// ==================== Making the debug messenger create info stuct ===================
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
		debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerCreateInfo.pNext = nullptr;
		debugMessengerCreateInfo.flags = 0;
		debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		//debugMessengerCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		//debugMessengerCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//debugMessengerCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		debugMessengerCreateInfo.pfnUserCallback = debugCallback;
		debugMessengerCreateInfo.pUserData = nullptr;
#endif // !GR_DIST

		// ================== Creating instance =================================
		{
			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifndef GR_DIST
			createInfo.pNext = &debugMessengerCreateInfo; // Allows debug msg on instace creation and destruction
#else
			createInfo.pNext = nullptr;
#endif // !GR_DIST
			createInfo.flags = 0;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledLayerCount = (u32)requiredLayers.Size();
			createInfo.ppEnabledLayerNames = (const char* const*)requiredLayers.GetRawElements();
			createInfo.enabledExtensionCount = (u32)requiredExtensions.Size();
			createInfo.ppEnabledExtensionNames = (const char* const*)requiredExtensions.GetRawElements();

			VkResult result = vkCreateInstance(&createInfo, state->allocator, &state->instance);

			requiredExtensions.Deinitialize();
			requiredLayers.Deinitialize();

			if (result != VK_SUCCESS)
			{
				GRFATAL("Failed to create Vulkan instance");
				GFree(state);
				state = nullptr;
				return false;
			}
		}

		// =============== Creating debug messenger ============================
#ifndef GR_DIST
		{
			PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state->instance, "vkCreateDebugUtilsMessengerEXT");

			if (VK_SUCCESS != vkCreateDebugUtilsMessengerEXT(state->instance, &debugMessengerCreateInfo, state->allocator, &state->debugMessenger))
			{
				GRFATAL("Failed to create Vulkan debug utils messenger");
				vkDestroyInstance(state->instance, nullptr);
				GFree(state);
				state = nullptr;
				return false;
			}
		}
#endif // !GR_DIST

		// ================ Getting a physical device ==============================
		{
			state->physicalDevice = VK_NULL_HANDLE;

			u32 deviceCount = 0;
			vkEnumeratePhysicalDevices(state->instance, &deviceCount, nullptr);
			if (deviceCount == 0)
			{
				GRFATAL("No Vulkan devices found");
				/// TODO: ifndef gr dist destroy debug utils messenger
				vkDestroyInstance(state->instance, nullptr);
				GFree(state);
				state = nullptr;
				return false;
			}

			Darray<VkPhysicalDevice> availableDevices = Darray<VkPhysicalDevice>();
			availableDevices.Initialize(MEM_TAG_RENDERER_SUBSYS, deviceCount);
			availableDevices.Size() = deviceCount;
			vkEnumeratePhysicalDevices(state->instance, &deviceCount, availableDevices.GetRawElements());

			/// TODO: better device selection
			for (u32 i = 0; i < availableDevices.Size(); ++i)
			{
				VkPhysicalDeviceProperties properties;
				vkGetPhysicalDeviceProperties(availableDevices[i], &properties);
				if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					state->physicalDevice = availableDevices[i];
					break;
				}
			}

			availableDevices.Deinitialize();
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

#ifndef GR_DIST
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state->instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(state->instance, state->debugMessenger, state->allocator);
#endif // !GR_DIST
		
		vkDestroyInstance(state->instance, nullptr);
		GFree(state);
		state = nullptr;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		const char* type;

		switch (messageType)
		{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			type = "general    ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			type = "validation ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			type = "performance";
			break;
		}

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			GRTRACE("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			GRTRACE("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			GRWARN("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			GRERROR("VK Validation, {}: {}", type, pCallbackData->pMessage);
			break;
		}

		return VK_FALSE;
	}
}