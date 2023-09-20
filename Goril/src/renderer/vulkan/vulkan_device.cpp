#include "vulkan_device.h"

#include "core/logger.h"
#include <cstring> ///TODO: remove

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	u32 formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	details.formatsDarray = (VkSurfaceFormatKHR*)DarrayCreateWithSize(sizeof(VkSurfaceFormatKHR), formatCount, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: replace allocator
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, (VkSurfaceFormatKHR*)details.formatsDarray);

	u32 presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	details.presentModesDarray = (VkPresentModeKHR*)DarrayCreateWithSize(sizeof(VkPresentModeKHR), presentModeCount, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: replace allocator
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, (VkPresentModeKHR*)details.presentModesDarray);

	return details;
}

static bool DeviceHasExtensions(VkPhysicalDevice physicalDevice, void** requiredDeviceExtensionsDarray)
{
	u32 availableExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
	VkExtensionProperties* availableExtensionsDarray = (VkExtensionProperties*)DarrayCreateWithSize(sizeof(VkExtensionProperties), availableExtensionCount, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: replace allocator
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensionsDarray);

	u32 availableRequiredExtensions = 0;
	for (u32 i = 0; i < DarrayGetSize(requiredDeviceExtensionsDarray); ++i)
	{
		for (u32 j = 0; j < availableExtensionCount; ++j)
		{
			if (0 == strncmp((const char*)requiredDeviceExtensionsDarray[i], availableExtensionsDarray[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE))
			{
				availableRequiredExtensions++;
			}
		}
	}

	DarrayDestroy(availableExtensionsDarray);

	return availableRequiredExtensions == DarrayGetSize(requiredDeviceExtensionsDarray);
}

bool SelectPhysicalDevice(void** requiredDeviceExtensionsDarray)
{
	vk_state->physicalDevice = VK_NULL_HANDLE;

	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(vk_state->instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		GRFATAL("No Vulkan devices found");
		return false;
	}

	VkPhysicalDevice* availableDevicesDarray = (VkPhysicalDevice*)DarrayCreateWithSize(sizeof(VkPhysicalDevice), deviceCount, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: different allocator
	vkEnumeratePhysicalDevices(vk_state->instance, &deviceCount, availableDevicesDarray);

	/// TODO: better device selection
	for (u32 i = 0; i < DarrayGetSize(availableDevicesDarray); ++i)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(availableDevicesDarray[i], &properties);
		bool isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		bool hasExtensions = DeviceHasExtensions(availableDevicesDarray[i], requiredDeviceExtensionsDarray);
		if (isDiscrete && hasExtensions)
		{
			GRINFO("Device with required extensions, features and properties found");
			SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(availableDevicesDarray[i], vk_state->surface);
			if (DarrayGetSize(swapchainSupport.formatsDarray) != 0 && DarrayGetSize(swapchainSupport.presentModesDarray) != 0)
			{
				vk_state->physicalDevice = availableDevicesDarray[i];
				vk_state->swapchainSupport = swapchainSupport;
				break;
			}
			DarrayDestroy(swapchainSupport.formatsDarray);
			DarrayDestroy(swapchainSupport.presentModesDarray);
		}
	}

	DarrayDestroy(availableDevicesDarray);

	if (vk_state->physicalDevice == VK_NULL_HANDLE)
	{
		GRFATAL("No suitable devices found");
		return false;
	}

	return true;
}

void SelectQueueFamilies(RendererState* state)
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, nullptr);
	VkQueueFamilyProperties* availableQueueFamiliesDarray = (VkQueueFamilyProperties*)DarrayCreateWithSize(sizeof(VkQueueFamilyProperties), queueFamilyCount, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);
	vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, availableQueueFamiliesDarray);

	state->queueIndices.transferFamily = UINT32_MAX;

	for (u32 i = 0; i < queueFamilyCount; ++i)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(state->physicalDevice, i, state->surface, &presentSupport);
		bool graphicsSupport = availableQueueFamiliesDarray[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
		bool transferSupport = availableQueueFamiliesDarray[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
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
	DarrayDestroy(availableQueueFamiliesDarray);
}

bool CreateLogicalDevice(RendererState* state, void** requiredDeviceExtensionsDarray, void** requiredDeviceLayersDarray)
{

	// ===================== Specifying queues for logical device =================================
	VkDeviceQueueCreateInfo* queueCreateInfosDarray = (VkDeviceQueueCreateInfo*)DarrayCreate(sizeof(VkDeviceQueueCreateInfo), 1, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: switch allocator

	u32* uniqueQueueFamiliesDarray = (u32*)DarrayCreate(sizeof(u32), 5, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS); // TODO: switch allocator
	if (!DarrayContains(uniqueQueueFamiliesDarray, &state->queueIndices.graphicsFamily))
		uniqueQueueFamiliesDarray = (u32*)DarrayPushback(uniqueQueueFamiliesDarray, &state->queueIndices.graphicsFamily);
	if (!DarrayContains(uniqueQueueFamiliesDarray, &state->queueIndices.presentFamily))
		uniqueQueueFamiliesDarray = (u32*)DarrayPushback(uniqueQueueFamiliesDarray, &state->queueIndices.presentFamily);
	if (!DarrayContains(uniqueQueueFamiliesDarray, &state->queueIndices.transferFamily))
		uniqueQueueFamiliesDarray = (u32*)DarrayPushback(uniqueQueueFamiliesDarray, &state->queueIndices.transferFamily);

	f32 queuePriority = 1.0f;

	for (u32 i = 0; i < DarrayGetSize(uniqueQueueFamiliesDarray); ++i)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueFamilyIndex = uniqueQueueFamiliesDarray[i];
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfosDarray = (VkDeviceQueueCreateInfo*)DarrayPushback(queueCreateInfosDarray, &queueCreateInfo);
	}

	DarrayDestroy(uniqueQueueFamiliesDarray);

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
	createInfo.queueCreateInfoCount = DarrayGetSize(queueCreateInfosDarray);
	createInfo.pQueueCreateInfos = queueCreateInfosDarray;
#ifndef GR_DIST
	createInfo.enabledLayerCount = DarrayGetSize(requiredDeviceLayersDarray);
	createInfo.ppEnabledLayerNames = (const char* const*)requiredDeviceLayersDarray;
#else
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
#endif // !GR_DIST
	createInfo.enabledExtensionCount = DarrayGetSize(requiredDeviceExtensionsDarray);
	createInfo.ppEnabledExtensionNames = (const char* const*)requiredDeviceExtensionsDarray;
	createInfo.pEnabledFeatures = nullptr;

	u32 result = vkCreateDevice(state->physicalDevice, &createInfo, state->allocator, &state->device);

	DarrayDestroy(queueCreateInfosDarray);

	if (result != VK_SUCCESS)
	{
		GRFATAL("Failed to create Vulkan logical device");
		return false;
	}

	return true;
}

void DestroyLogicalDevice(RendererState* state)
{
	if (state->device)
		vkDestroyDevice(state->device, state->allocator);
	if (state->swapchainSupport.formatsDarray)
		DarrayDestroy(state->swapchainSupport.formatsDarray);
	if (state->swapchainSupport.presentModesDarray)
		DarrayDestroy(state->swapchainSupport.presentModesDarray);
}
