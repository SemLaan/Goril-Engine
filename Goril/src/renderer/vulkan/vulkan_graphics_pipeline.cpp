#include "vulkan_graphics_pipeline.h"

#include "vulkan_shader_loader.h"
#include "../buffer.h"
#include "vulkan_buffer.h"

namespace GR
{
	b8 CreateGraphicsPipeline()
	{
		// Creating shader modules
		Darray<char> vert;
		Darray<char> frag;
		ReadFile("C:/Users/semla/Documents/Git_Repos/zelf_projecten/c++/Goril/Goril/src/renderer/shaders/vershader.spv", MEM_TAG_RENDERER_SUBSYS, &vert);
		ReadFile("C:/Users/semla/Documents/Git_Repos/zelf_projecten/c++/Goril/Goril/src/renderer/shaders/frshader.spv", MEM_TAG_RENDERER_SUBSYS, &frag);
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		CreateShaderModule(vk_state, vert, &vertShaderModule);
		CreateShaderModule(vk_state, frag, &fragShaderModule);
		vert.Deinitialize();
		frag.Deinitialize();

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext = nullptr;
		descriptorSetLayoutCreateInfo.flags = 0;
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.pBindings = &uboLayoutBinding;

		if (VK_SUCCESS != vkCreateDescriptorSetLayout(vk_state->device, &descriptorSetLayoutCreateInfo, vk_state->allocator, &vk_state->descriptorSetLayout))
		{
			GRFATAL("Vulkan descriptor set layout creation failed");
			vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
			vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
			return false;
		}

		VkDeviceSize uniformBufferSize = sizeof(GlobalUniformObject);
		
		vk_state->uniformBuffers = CreateDarrayWithSize<VkBuffer>(MEM_TAG_RENDERER_SUBSYS, vk_state->maxFramesInFlight);
		vk_state->uniformBuffersMemory = CreateDarrayWithSize<VkDeviceMemory>(MEM_TAG_RENDERER_SUBSYS, vk_state->maxFramesInFlight);
		vk_state->uniformBuffersMapped = CreateDarrayWithSize<void*>(MEM_TAG_RENDERER_SUBSYS, vk_state->maxFramesInFlight);

		for (i32 i = 0; i < vk_state->maxFramesInFlight; ++i)
		{
			CreateBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vk_state->uniformBuffers[i], &vk_state->uniformBuffersMemory[i]);
			vkMapMemory(vk_state->device, vk_state->uniformBuffersMemory[i], 0, uniformBufferSize, 0, &vk_state->uniformBuffersMapped[i]);
		}

		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSize.descriptorCount = (u32)vk_state->maxFramesInFlight;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = (u32)vk_state->maxFramesInFlight;
		descriptorPoolCreateInfo.poolSizeCount = 1;
		descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

		if (VK_SUCCESS != vkCreateDescriptorPool(vk_state->device, &descriptorPoolCreateInfo, vk_state->allocator, &vk_state->uniformDescriptorPool))
		{
			GRFATAL("Vulkan descriptor pool creation failed");
			vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
			vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
			return false;
		}

		Darray<VkDescriptorSetLayout> descriptorSetLayouts = CreateDarrayWithCapacity<VkDescriptorSetLayout>(MEM_TAG_RENDERER_SUBSYS, (u32)vk_state->maxFramesInFlight);
		for (i32 i = 0; i < vk_state->maxFramesInFlight; ++i)
		{
			descriptorSetLayouts.Pushback(vk_state->descriptorSetLayout);
		}

		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.pNext = nullptr;
		descriptorSetAllocInfo.descriptorPool = vk_state->uniformDescriptorPool;
		descriptorSetAllocInfo.descriptorSetCount = (u32)vk_state->maxFramesInFlight;
		descriptorSetAllocInfo.pSetLayouts = descriptorSetLayouts.GetRawElements();

		vk_state->uniformDescriptorSets = CreateDarrayWithSize<VkDescriptorSet>(MEM_TAG_RENDERER_SUBSYS, (u32)vk_state->maxFramesInFlight);

		if (VK_SUCCESS != vkAllocateDescriptorSets(vk_state->device, &descriptorSetAllocInfo, vk_state->uniformDescriptorSets.GetRawElements()))
		{
			GRFATAL("Vulkan descriptor set allocation failed");
			descriptorSetLayouts.Deinitialize();
			vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
			vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
			return false;
		}

		descriptorSetLayouts.Deinitialize();

		for (i32 i = 0; i < vk_state->maxFramesInFlight; ++i)
		{
			VkDescriptorBufferInfo descriptorBufferInfo{};
			descriptorBufferInfo.buffer = vk_state->uniformBuffers[i];
			descriptorBufferInfo.offset = 0;
			descriptorBufferInfo.range = sizeof(GlobalUniformObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.pNext = nullptr;
			descriptorWrite.dstSet = vk_state->uniformDescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pBufferInfo = &descriptorBufferInfo;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(vk_state->device, 1, &descriptorWrite, 0, nullptr);
		}

		VkPipelineShaderStageCreateInfo vertStageCreateInfo{};
		vertStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertStageCreateInfo.pNext = nullptr;
		vertStageCreateInfo.flags = 0;
		vertStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertStageCreateInfo.module = vertShaderModule;
		vertStageCreateInfo.pName = "main";
		vertStageCreateInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragStageCreateInfo{};
		fragStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragStageCreateInfo.pNext = nullptr;
		fragStageCreateInfo.flags = 0;
		fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageCreateInfo.module = fragShaderModule;
		fragStageCreateInfo.pName = "main";
		fragStageCreateInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[2] = { vertStageCreateInfo, fragStageCreateInfo };

		// Dynamic states
		constexpr u32 dynamicStateCount = 2;
		VkDynamicState dynamicStates[dynamicStateCount] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.pNext = nullptr;
		dynamicStateCreateInfo.flags = 0;
		dynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
		dynamicStateCreateInfo.pDynamicStates = dynamicStates;

		// Vertex input
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attributeDescriptions[2]{};
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.pNext = nullptr;
		vertexInputCreateInfo.flags = 0;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 2;
		vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;

		// Input assembler
		VkPipelineInputAssemblyStateCreateInfo inputAssemblerCreateInfo{};
		inputAssemblerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblerCreateInfo.pNext = nullptr;
		inputAssemblerCreateInfo.flags = 0;
		inputAssemblerCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblerCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.pNext = nullptr;
		viewportStateCreateInfo.flags = 0;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = nullptr; // dynamic state
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = nullptr;  // dynamic state

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
		rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerCreateInfo.pNext = nullptr;
		rasterizerCreateInfo.flags = 0;
		rasterizerCreateInfo.depthClampEnable = VK_FALSE;
		rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizerCreateInfo.depthBiasClamp = 0.0f;
		rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;
		rasterizerCreateInfo.lineWidth = 1.0f;

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
		multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingCreateInfo.pNext = nullptr;
		multisamplingCreateInfo.flags = 0;
		multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
		multisamplingCreateInfo.minSampleShading = 1.0f;
		multisamplingCreateInfo.pSampleMask = nullptr;
		multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

		// Depth/stencil
		/// TODO: add depth stencil state create info

		// Blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo blendStateCreateInfo{};
		blendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendStateCreateInfo.pNext = nullptr;
		blendStateCreateInfo.flags = 0;
		blendStateCreateInfo.logicOpEnable = VK_FALSE;
		blendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		blendStateCreateInfo.attachmentCount = 1;
		blendStateCreateInfo.pAttachments = &colorBlendAttachment;

		// Pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &vk_state->descriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		if (VK_SUCCESS != vkCreatePipelineLayout(vk_state->device, &pipelineLayoutCreateInfo, vk_state->allocator, &vk_state->pipelineLayout))
		{
			GRFATAL("Vulkan pipeline layout creation failed");
			vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
			vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
			return false;
		}

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext = nullptr;
		graphicsPipelineCreateInfo.flags = 0;
		graphicsPipelineCreateInfo.stageCount = 2;
		graphicsPipelineCreateInfo.pStages = shaderStagesCreateInfo;
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblerCreateInfo;
		graphicsPipelineCreateInfo.pTessellationState = nullptr;
		graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
		graphicsPipelineCreateInfo.pColorBlendState = &blendStateCreateInfo;
		graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
		graphicsPipelineCreateInfo.layout = vk_state->pipelineLayout;
		graphicsPipelineCreateInfo.renderPass = vk_state->renderpass;
		graphicsPipelineCreateInfo.subpass = 0;
		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		if (VK_SUCCESS != vkCreateGraphicsPipelines(vk_state->device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, vk_state->allocator, &vk_state->graphicsPipeline))
		{
			GRFATAL("Vulkan graphics pipeline creation failed");
			vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
			vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
			return false;
		}

		vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
		vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);

		return true;
	}

	void DestroyGraphicsPipeline()
	{
		if (vk_state->graphicsPipeline)
			vkDestroyPipeline(vk_state->device, vk_state->graphicsPipeline, vk_state->allocator);
		if (vk_state->pipelineLayout)
			vkDestroyPipelineLayout(vk_state->device, vk_state->pipelineLayout, vk_state->allocator);

		for (i32 i = 0; i < vk_state->maxFramesInFlight; ++i)
		{
			vkUnmapMemory(vk_state->device, vk_state->uniformBuffersMemory[i]);
			vkDestroyBuffer(vk_state->device, vk_state->uniformBuffers[i], vk_state->allocator);
			vkFreeMemory(vk_state->device, vk_state->uniformBuffersMemory[i], vk_state->allocator);
		}

		vk_state->uniformBuffers.Deinitialize();
		vk_state->uniformBuffersMapped.Deinitialize();
		vk_state->uniformBuffersMemory.Deinitialize();
		vk_state->uniformDescriptorSets.Deinitialize();

		if (vk_state->uniformDescriptorPool)
			vkDestroyDescriptorPool(vk_state->device, vk_state->uniformDescriptorPool, vk_state->allocator);

		if (vk_state->descriptorSetLayout)
			vkDestroyDescriptorSetLayout(vk_state->device, vk_state->descriptorSetLayout, vk_state->allocator);
	}
}