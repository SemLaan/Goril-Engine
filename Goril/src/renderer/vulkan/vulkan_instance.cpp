#include "vulkan_instance.h"

#include "vulkan_debug_messenger.h"

namespace GR
{
	b8 CreateVulkanInstance(const Darray<const void*>& requiredExtensions, const Darray<const void*>& requiredLayers)
	{
		// ================ App info =============================================
		VkApplicationInfo appInfo{};
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
			Darray<VkExtensionProperties> availableExtensions = CreateDarrayWithSize<VkExtensionProperties>(MEM_TAG_RENDERER_SUBSYS, availableExtensionCount);
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
			Darray<VkLayerProperties> availableLayers = CreateDarrayWithSize<VkLayerProperties>(MEM_TAG_RENDERER_SUBSYS, availableLayerCount);
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
				return false;
			}
			else
				GRTRACE("Required Vulkan layers found");
		}

		// Enabling and disabling certain validation features
		VkValidationFeatureEnableEXT enableSynchronizationValidationFeature = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;

		VkValidationFeaturesEXT validationFeatures{};
		validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		validationFeatures.pNext = nullptr;
		validationFeatures.enabledValidationFeatureCount = 1;
		validationFeatures.pEnabledValidationFeatures = &enableSynchronizationValidationFeature;
		validationFeatures.disabledValidationFeatureCount = 0;
		validationFeatures.pDisabledValidationFeatures = nullptr;

#endif // !GR_DIST

		// ================== Creating instance =================================
		{
			VkInstanceCreateInfo createInfo{};
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
			createInfo.enabledLayerCount = (u32)requiredLayers.Size();
			createInfo.ppEnabledLayerNames = (const char* const*)requiredLayers.GetRawElements();
			createInfo.enabledExtensionCount = (u32)requiredExtensions.Size();
			createInfo.ppEnabledExtensionNames = (const char* const*)requiredExtensions.GetRawElements();

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
}