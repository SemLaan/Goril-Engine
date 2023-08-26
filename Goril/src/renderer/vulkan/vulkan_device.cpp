#include "vulkan_device.h"

#include "core/logger.h"

namespace GR
{
	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) 
	{
		SwapchainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		u32 formatCount = 0; 
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		details.formats = CreateDarrayWithSize<VkSurfaceFormatKHR>(MEM_TAG_RENDERER_SUBSYS, formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.GetRawElements());

		u32 presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		details.presentModes = CreateDarrayWithSize<VkPresentModeKHR>(MEM_TAG_RENDERER_SUBSYS, presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.GetRawElements());

		return details;
	}

	static b8 DeviceHasExtensions(VkPhysicalDevice physicalDevice, const Darray<const void*>* requiredDeviceExtensions)
	{
		u32 availableExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
		Darray<VkExtensionProperties> availableExtensions = CreateDarrayWithSize<VkExtensionProperties>(MEM_TAG_RENDERER_SUBSYS, availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.GetRawElements());

		u32 availableRequiredExtensions = 0;
		for (u32 i = 0; i < requiredDeviceExtensions->Size(); ++i)
		{
			for (u32 j = 0; j < availableExtensionCount; ++j)
			{
				if (0 == strncmp((const char*)(*requiredDeviceExtensions)[i], availableExtensions[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE))
				{
					availableRequiredExtensions++;
				}
			}
		}

		availableExtensions.Deinitialize();

		return availableRequiredExtensions == requiredDeviceExtensions->Size();
	}

	b8 SelectPhysicalDevice(RendererState* state, const Darray<const void*>* requiredDeviceExtensions)
	{
		state->physicalDevice = VK_NULL_HANDLE;

		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(state->instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			GRFATAL("No Vulkan devices found");
			return false;
		}

		Darray<VkPhysicalDevice> availableDevices = CreateDarrayWithSize<VkPhysicalDevice>(MEM_TAG_RENDERER_SUBSYS, deviceCount);
		vkEnumeratePhysicalDevices(state->instance, &deviceCount, availableDevices.GetRawElements());

		/// TODO: better device selection
		for (u32 i = 0; i < availableDevices.Size(); ++i)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(availableDevices[i], &properties);
			b8 isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			b8 hasExtensions = DeviceHasExtensions(availableDevices[i], requiredDeviceExtensions);
			if (isDiscrete && hasExtensions)
			{
				GRINFO("Device with required extensions, features and properties found");
				SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(availableDevices[i], state->surface);
				if (swapchainSupport.formats.Size() != 0 && swapchainSupport.presentModes.Size() != 0)
				{
					state->physicalDevice = availableDevices[i];
					state->swapchainSupport = swapchainSupport;
					break;
				}
				swapchainSupport.formats.Deinitialize();
				swapchainSupport.presentModes.Deinitialize();
			}
		}

		availableDevices.Deinitialize();

		return true;
	}

	void SelectQueueFamilies(RendererState* state)
	{
		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, nullptr);
		Darray<VkQueueFamilyProperties> availableQueueFamilies = CreateDarrayWithSize<VkQueueFamilyProperties>(MEM_TAG_RENDERER_SUBSYS, queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, availableQueueFamilies.GetRawElements());

		state->queueIndices.transferFamily = UINT32_MAX;

		for (u32 i = 0; i < queueFamilyCount; ++i)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(state->physicalDevice, i, state->surface, &presentSupport);
			b8 graphicsSupport = availableQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
			b8 transferSupport = availableQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
			if (graphicsSupport)
				state->queueIndices.graphicsFamily = i;
			if (presentSupport)
				state->queueIndices.presentFamily = i;
			if (transferSupport && !graphicsSupport)
				state->queueIndices.transferFamily = i;
		}

		if (state->queueIndices.transferFamily == UINT32_MAX)
			state->queueIndices.transferFamily = state->queueIndices.graphicsFamily;
		/// TODO: check if the device even has queue families for all these things, if not fail startup (is this even required? i think implementations need at least transfer and graphics(?), and compute and present are implied by the existence of the extensions)
		availableQueueFamilies.Deinitialize();
	}

	b8 CreateLogicalDevice(RendererState* state, const Darray<const void*>* requiredDeviceExtensions, const Darray<const void*>* requiredDeviceLayers)
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
		if (!uniqueQueueFamilies.Contains(state->queueIndices.transferFamily))
			uniqueQueueFamilies.Pushback(state->queueIndices.transferFamily);

		f32 queuePriority = 1.0f;

		for (u32 i = 0; i < uniqueQueueFamilies.Size(); ++i)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
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
		VkPhysicalDeviceFeatures deviceFeatures{};
		/// TODO: add required device features here, these should be retrieved from the application config

		/// Put new extension features above here and make the extension feature under this point to that new feature
		VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{};
		timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
		timelineSemaphoreFeatures.pNext = nullptr;
		timelineSemaphoreFeatures.timelineSemaphore = VK_TRUE;

		VkPhysicalDeviceSynchronization2Features synchronization2Features{};
		synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		synchronization2Features.pNext = &timelineSemaphoreFeatures;
		synchronization2Features.synchronization2 = VK_TRUE;

		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &synchronization2Features;
		deviceFeatures2.features = deviceFeatures;

		// ===================== Creating logical device =============================================
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &deviceFeatures2;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = (u32)queueCreateInfos.Size();
		createInfo.pQueueCreateInfos = queueCreateInfos.GetRawElements();
#ifndef GR_DIST
		createInfo.enabledLayerCount = (u32)requiredDeviceLayers->Size();
		createInfo.ppEnabledLayerNames = (const char* const*)requiredDeviceLayers->GetRawElements();
#else
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
#endif // !GR_DIST
		createInfo.enabledExtensionCount = (u32)requiredDeviceExtensions->Size();
		createInfo.ppEnabledExtensionNames = (const char* const*)requiredDeviceExtensions->GetRawElements();
		createInfo.pEnabledFeatures = nullptr;

		u32 result = vkCreateDevice(state->physicalDevice, &createInfo, state->allocator, &state->device);

		queueCreateInfos.Deinitialize();

		if (result != VK_SUCCESS)
		{
			GRFATAL("Failed to create Vulkan logical device");
			return false;
		}

		// =================== Getting the device queues ======================================================
		// Present family queue
		vkGetDeviceQueue(state->device, state->queueIndices.presentFamily, 0, &state->presentQueue);

		///TODO: get compute queue
		// Graphics, transfer and (in the future) compute queue
		vkGetDeviceQueue(state->device, state->queueIndices.graphicsFamily, 0, &state->graphicsQueue.handle);
		state->graphicsQueue.index = state->queueIndices.graphicsFamily;

		vkGetDeviceQueue(state->device, state->queueIndices.transferFamily, 0, &state->transferQueue.handle);
		state->transferQueue.index = state->queueIndices.transferFamily;

		// ==================== Creating command pools for each of the queue families =============================
		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = vk_state->queueIndices.graphicsFamily;

		if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->allocator, &vk_state->graphicsQueue.commandPool))
		{
			GRFATAL("Failed to create Vulkan graphics command pool");
			return false;
		}

		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolCreateInfo.queueFamilyIndex = vk_state->queueIndices.transferFamily;

		if (VK_SUCCESS != vkCreateCommandPool(vk_state->device, &commandPoolCreateInfo, vk_state->allocator, &vk_state->transferQueue.commandPool))
		{
			GRFATAL("Failed to create Vulkan transfer command pool");
			return false;
		}

		///TODO: create compute command pool

		return true;
	}

	void DestroyLogicalDevice(RendererState* state)
	{
		if (vk_state->graphicsQueue.commandPool)
			vkDestroyCommandPool(vk_state->device, vk_state->graphicsQueue.commandPool, vk_state->allocator);

		if (vk_state->transferQueue.commandPool)
			vkDestroyCommandPool(vk_state->device, vk_state->transferQueue.commandPool, vk_state->allocator);

		if (state->device)
			vkDestroyDevice(state->device, state->allocator);
		if (state->swapchainSupport.formats.GetRawElements())
			state->swapchainSupport.formats.Deinitialize();
		if (state->swapchainSupport.presentModes.GetRawElements())
			state->swapchainSupport.presentModes.Deinitialize();
	}
}