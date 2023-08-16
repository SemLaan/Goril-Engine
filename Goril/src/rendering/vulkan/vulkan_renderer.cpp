#include "../renderer.h"

#include "core/logger.h"
#include "core/gr_memory.h"
#include "containers/darray.h"

#include "vulkan_platform.h"
#include "vulkan_types.h"
#include "vulkan_debug_messenger.h"
#include "vulkan_instance.h"

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
		GetPlatformExtensions(&requiredDeviceExtensions);
		requiredDeviceExtensions.Pushback(&VK_KHR_SURFACE_EXTENSION_NAME);
		requiredDeviceExtensions.Deinitialize();
		{
			state->physicalDevice = VK_NULL_HANDLE;

			u32 deviceCount = 0;
			vkEnumeratePhysicalDevices(state->instance, &deviceCount, nullptr);
			if (deviceCount == 0)
			{
				GRFATAL("No Vulkan devices found");
				requiredLayers.Deinitialize();
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
				b8 isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
				if (isDiscrete)
				{
					state->physicalDevice = availableDevices[i];
					break;
				}
			}

			availableDevices.Deinitialize();
		}

		// ================== Getting device queue families ==============================
		{
			u32 queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, nullptr);
			Darray<VkQueueFamilyProperties> availableQueueFamilies = Darray<VkQueueFamilyProperties>();
			availableQueueFamilies.Initialize(MEM_TAG_RENDERER_SUBSYS, queueFamilyCount);
			availableQueueFamilies.Size() = queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, availableQueueFamilies.GetRawElements());

			for (u32 i = 0; i < queueFamilyCount; ++i)
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(state->physicalDevice, i, state->surface, &presentSupport);
				b8 graphicsSupport = availableQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
				if (graphicsSupport)
					state->queueIndices.graphicsFamily = i;
				if (presentSupport)
					state->queueIndices.presentFamily = i;
			}
			/// TODO: check if the device even has queue families for all these things, if not fail startup
			availableQueueFamilies.Deinitialize();
		}

		{
			// ===================== Specifying queues for logical device =================================
			Darray<VkDeviceQueueCreateInfo> queueCreateInfos = Darray<VkDeviceQueueCreateInfo>();
			queueCreateInfos.Initialize(MEM_TAG_RENDERER_SUBSYS);

			Darray<u32> uniqueQueueFamilies = Darray<u32>();
			uniqueQueueFamilies.Initialize(MEM_TAG_RENDERER_SUBSYS);
			if (!uniqueQueueFamilies.Contains(state->queueIndices.graphicsFamily))
				uniqueQueueFamilies.Pushback(state->queueIndices.graphicsFamily);
			if (!uniqueQueueFamilies.Contains(state->queueIndices.presentFamily))
				uniqueQueueFamilies.Pushback(state->queueIndices.presentFamily);

			f32 queuePriority = 1.0f;

			for (u32 i = 0; i < uniqueQueueFamilies.Size(); ++i)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.pNext = nullptr;
				queueCreateInfo.flags = 0;
				queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.Pushback(queueCreateInfo);
			}

			uniqueQueueFamilies.Deinitialize();

			// ===================== Specifying features for logical device ==============================
			VkPhysicalDeviceFeatures deviceFeatures = {};
			/// TODO: add required device features here, these should be retrieved from the application config

			// ===================== Creating logical device =============================================
			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.queueCreateInfoCount = (u32)queueCreateInfos.Size();
			createInfo.pQueueCreateInfos = queueCreateInfos.GetRawElements();
#ifndef GR_DIST
			createInfo.enabledLayerCount = (u32)requiredLayers.Size();
			createInfo.ppEnabledLayerNames = (const char* const*)requiredLayers.GetRawElements();
#else
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
#endif // !GR_DIST
			createInfo.enabledExtensionCount = 0;
			createInfo.ppEnabledExtensionNames = nullptr;
			createInfo.pEnabledFeatures = &deviceFeatures;

			u32 result = vkCreateDevice(state->physicalDevice, &createInfo, state->allocator, &state->device);

			requiredLayers.Deinitialize();
			queueCreateInfos.Deinitialize();

			if (result != VK_SUCCESS)
			{
				GRFATAL("Failed to create Vulkan logical device");
				return false;
			}
		}

		// =================== Getting the device queues ======================================================
		vkGetDeviceQueue(state->device, state->queueIndices.graphicsFamily, 0, &state->graphicsQueue);
		vkGetDeviceQueue(state->device, state->queueIndices.presentFamily, 0, &state->presentQueue);

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

		// ===================== Destroying logical device if it was created =================================
		if (state->device)
			vkDestroyDevice(state->device, state->allocator);

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