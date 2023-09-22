#include "vulkan_instance.h"

#include "vulkan_debug_messenger.h"
#include "core/logger.h"
#include <string.h>

bool CreateVulkanInstance(void** requiredExtensionsDarray, void** requiredLayersDarray)
{
	// ================ App info =============================================
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Test app";
	appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.pEngineName = "Goril";
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	{
		// Checking if required extensions are available
		u32 availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		VkExtensionProperties* availableExtensionsDarray = (VkExtensionProperties*)DarrayCreateWithSize(sizeof(VkExtensionProperties), availableExtensionCount, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensionsDarray);

		u32 availableRequiredExtensions = 0;
		for (u32 i = 0; i < DarrayGetSize(requiredExtensionsDarray); ++i)
		{
			for (u32 j = 0; j < availableExtensionCount; ++j)
			{
				if (0 == strncmp((const char*)requiredExtensionsDarray[i], availableExtensionsDarray[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE))
				{
					availableRequiredExtensions++;
				}
			}
		}

		DarrayDestroy(availableExtensionsDarray);

		if (availableRequiredExtensions < DarrayGetSize(requiredExtensionsDarray))
		{
			GRFATAL("Couldn't find required Vulkan extensions");
			return false;
		}
		else
			GRTRACE("Required Vulkan extensions found");
	}

#ifndef GR_DIST

	{
		// Checking if required layers are available
		u32 availableLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
		VkLayerProperties* availableLayersDarray = (VkLayerProperties*)DarrayCreate(sizeof(VkLayerProperties), availableLayerCount, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);
		vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayersDarray);

		u32 availableRequiredLayers = 0;
		for (u32 i = 0; i < DarrayGetSize(requiredLayersDarray); ++i)
		{
			for (u32 j = 0; j < availableLayerCount; ++j)
			{
				if (0 == strncmp((const char*)requiredLayersDarray[i], availableLayersDarray[j].layerName, VK_MAX_EXTENSION_NAME_SIZE))
				{
					availableRequiredLayers++;
				}
			}
		}

		DarrayDestroy(availableLayersDarray);

		if (availableRequiredLayers < DarrayGetSize(requiredLayersDarray))
		{
			GRFATAL("Couldn't find required Vulkan layers");
			return false;
		}
		else
			GRTRACE("Required Vulkan layers found");
	}

	// Enabling and disabling certain validation features
	VkValidationFeatureEnableEXT enableSynchronizationValidationFeature = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;

	VkValidationFeaturesEXT validationFeatures = {};
	validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validationFeatures.pNext = nullptr;
	validationFeatures.enabledValidationFeatureCount = 1;
	validationFeatures.pEnabledValidationFeatures = &enableSynchronizationValidationFeature;
	validationFeatures.disabledValidationFeatureCount = 0;
	validationFeatures.pDisabledValidationFeatures = nullptr;

#endif // !GR_DIST

	// ================== Creating instance =================================
	{
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifndef GR_DIST
		VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = GetDebugMessengerCreateInfo();
		messengerCreateInfo.pNext = &validationFeatures;
		createInfo.pNext = &messengerCreateInfo; // Allows debug msg on instace creation and destruction
#else
		createInfo.pNext = nullptr;
#endif // !GR_DIST
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = DarrayGetSize(requiredLayersDarray);
		createInfo.ppEnabledLayerNames = (const char* const*)requiredLayersDarray;
		createInfo.enabledExtensionCount = (u32)DarrayGetSize(requiredExtensionsDarray);
		createInfo.ppEnabledExtensionNames = (const char* const*)requiredExtensionsDarray;

		VkResult result = vkCreateInstance(&createInfo, vk_state->allocator, &vk_state->instance);

		if (result != VK_SUCCESS)
		{
			GRFATAL("Failed to create Vulkan instance");
			return false;
		}
	}

	return true;
}

void DestroyVulkanInstance()
{
	if (vk_state->instance)
		vkDestroyInstance(vk_state->instance, vk_state->allocator);
}
