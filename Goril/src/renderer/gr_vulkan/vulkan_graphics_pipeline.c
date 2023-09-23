#include "vulkan_graphics_pipeline.h"

#include "vulkan_shader_loader.h"
#include "../buffer.h"
#include "vulkan_buffer.h"
#include "core/logger.h"
#include "../renderer_types.h"

bool CreateGraphicsPipeline()
{
	// Creating shader modules
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	CreateShaderModule("vershader.spv", vk_state, &vertShaderModule);
	CreateShaderModule("frshader.spv", vk_state, &fragShaderModule);

	VkDescriptorSetLayoutBinding uboLayoutBindings[2] = {};
	uboLayoutBindings[0].binding = 0;
	uboLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindings[0].descriptorCount = 1;
	uboLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBindings[0].pImmutableSamplers = nullptr;

	uboLayoutBindings[1].binding = 1;
	uboLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uboLayoutBindings[1].descriptorCount = 1;
	uboLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	uboLayoutBindings[1].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = nullptr;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = 2;
	descriptorSetLayoutCreateInfo.pBindings = uboLayoutBindings;

	if (VK_SUCCESS != vkCreateDescriptorSetLayout(vk_state->device, &descriptorSetLayoutCreateInfo, vk_state->allocator, &vk_state->descriptorSetLayout))
	{
		GRFATAL("Vulkan descriptor set layout creation failed");
		vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
		vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
		return false;
	}

	VkDeviceSize uniformBufferSize = sizeof(GlobalUniformObject);

	vk_state->uniformBuffersDarray = (VkBuffer*)DarrayCreateWithSize(sizeof(VkBuffer), MAX_FRAMES_IN_FLIGHT, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);
	vk_state->uniformBuffersMemoryDarray = (VkDeviceMemory*)DarrayCreateWithSize(sizeof(VkDeviceMemory), MAX_FRAMES_IN_FLIGHT, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);
	vk_state->uniformBuffersMappedDarray = (void**)DarrayCreateWithSize(sizeof(void*), MAX_FRAMES_IN_FLIGHT, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);

	for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		CreateBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vk_state->uniformBuffersDarray[i], &vk_state->uniformBuffersMemoryDarray[i]);
		vkMapMemory(vk_state->device, vk_state->uniformBuffersMemoryDarray[i], 0, uniformBufferSize, 0, &vk_state->uniformBuffersMappedDarray[i]);
	}

	VkDescriptorPoolSize descriptorPoolSizes[2] = {};
	descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
	descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
	descriptorPoolCreateInfo.poolSizeCount = 2;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;

	if (VK_SUCCESS != vkCreateDescriptorPool(vk_state->device, &descriptorPoolCreateInfo, vk_state->allocator, &vk_state->uniformDescriptorPool))
	{
		GRFATAL("Vulkan descriptor pool creation failed");
		vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
		vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
		return false;
	}

	VkDescriptorSetLayout* descriptorSetLayoutsDarray = (VkDescriptorSetLayout*)DarrayCreateWithSize(sizeof(VkDescriptorSetLayout), MAX_FRAMES_IN_FLIGHT, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);
	for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		descriptorSetLayoutsDarray[i] = vk_state->descriptorSetLayout;
	}

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.pNext = nullptr;
	descriptorSetAllocInfo.descriptorPool = vk_state->uniformDescriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
	descriptorSetAllocInfo.pSetLayouts = descriptorSetLayoutsDarray;

	vk_state->uniformDescriptorSetsDarray = (VkDescriptorSet*)DarrayCreateWithSize(sizeof(VkDescriptorSet), MAX_FRAMES_IN_FLIGHT, GetGlobalAllocator(), MEM_TAG_RENDERER_SUBSYS);

	if (VK_SUCCESS != vkAllocateDescriptorSets(vk_state->device, &descriptorSetAllocInfo, vk_state->uniformDescriptorSetsDarray))
	{
		GRFATAL("Vulkan descriptor set allocation failed");
		DarrayDestroy(descriptorSetLayoutsDarray);
		vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
		vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
		return false;
	}

	DarrayDestroy(descriptorSetLayoutsDarray);

	VkPipelineShaderStageCreateInfo vertStageCreateInfo = {};
	vertStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageCreateInfo.pNext = nullptr;
	vertStageCreateInfo.flags = 0;
	vertStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageCreateInfo.module = vertShaderModule;
	vertStageCreateInfo.pName = "main";
	vertStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragStageCreateInfo = {};
	fragStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageCreateInfo.pNext = nullptr;
	fragStageCreateInfo.flags = 0;
	fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageCreateInfo.module = fragShaderModule;
	fragStageCreateInfo.pName = "main";
	fragStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[2] = { vertStageCreateInfo, fragStageCreateInfo };

	// Dynamic states
	#define DYNAMIC_STATE_COUNT 2
	VkDynamicState dynamicStates[DYNAMIC_STATE_COUNT] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.flags = 0;
	dynamicStateCreateInfo.dynamicStateCount = DYNAMIC_STATE_COUNT;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	// Vertex input
	VkVertexInputBindingDescription vertexBindingDescription = {};
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = sizeof(Vertex);
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	#define VERTEX_ATTRIBUTE_COUNT 8
	VkVertexInputAttributeDescription attributeDescriptions[VERTEX_ATTRIBUTE_COUNT] = {};
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	// Vertex per instance input
	VkVertexInputBindingDescription instanceBindingDescription = {};
	instanceBindingDescription.binding = 1;
	instanceBindingDescription.stride = sizeof(SpriteInstance);
	instanceBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	// These nex four attributes are one matrix
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].binding = 1;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(SpriteInstance, model);
	//
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].binding = 1;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(SpriteInstance, model) + sizeof(f32) * 4;
	//
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].binding = 1;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[5].offset = offsetof(SpriteInstance, model) + sizeof(f32) * 8;
	//
	attributeDescriptions[6].location = 6;
	attributeDescriptions[6].binding = 1;
	attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[6].offset = offsetof(SpriteInstance, model) + sizeof(f32) * 12;

	attributeDescriptions[7].location = 7;
	attributeDescriptions[7].binding = 1;
	attributeDescriptions[7].format = VK_FORMAT_R32_UINT;
	attributeDescriptions[7].offset = offsetof(SpriteInstance, textureIndex);


	VkVertexInputBindingDescription bindingDescriptions[2] = {vertexBindingDescription, instanceBindingDescription};

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 2;
	vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT;
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;

	// Input assembler
	VkPipelineInputAssemblyStateCreateInfo inputAssemblerCreateInfo = {};
	inputAssemblerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblerCreateInfo.pNext = nullptr;
	inputAssemblerCreateInfo.flags = 0;
	inputAssemblerCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblerCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.pNext = nullptr;
	viewportStateCreateInfo.flags = 0;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = nullptr; // dynamic state
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = nullptr;  // dynamic state

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.pNext = nullptr;
	rasterizerCreateInfo.flags = 0;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizerCreateInfo.depthBiasClamp = 0.0f;
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;
	rasterizerCreateInfo.lineWidth = 1.0f;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
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
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo blendStateCreateInfo = {};
	blendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendStateCreateInfo.pNext = nullptr;
	blendStateCreateInfo.flags = 0;
	blendStateCreateInfo.logicOpEnable = VK_FALSE;
	blendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	blendStateCreateInfo.attachmentCount = 1;
	blendStateCreateInfo.pAttachments = &colorBlendAttachment;

	// Push constants
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantObject);

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &vk_state->descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	if (VK_SUCCESS != vkCreatePipelineLayout(vk_state->device, &pipelineLayoutCreateInfo, vk_state->allocator, &vk_state->pipelineLayout))
	{
		GRFATAL("Vulkan pipeline layout creation failed");
		vkDestroyShaderModule(vk_state->device, vertShaderModule, vk_state->allocator);
		vkDestroyShaderModule(vk_state->device, fragShaderModule, vk_state->allocator);
		return false;
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
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

	for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkUnmapMemory(vk_state->device, vk_state->uniformBuffersMemoryDarray[i]);
		vkDestroyBuffer(vk_state->device, vk_state->uniformBuffersDarray[i], vk_state->allocator);
		vkFreeMemory(vk_state->device, vk_state->uniformBuffersMemoryDarray[i], vk_state->allocator);
	}

	DarrayDestroy(vk_state->uniformBuffersDarray);
	DarrayDestroy(vk_state->uniformBuffersMappedDarray);
	DarrayDestroy(vk_state->uniformBuffersMemoryDarray);
	DarrayDestroy(vk_state->uniformDescriptorSetsDarray);

	if (vk_state->uniformDescriptorPool)
		vkDestroyDescriptorPool(vk_state->device, vk_state->uniformDescriptorPool, vk_state->allocator);

	if (vk_state->descriptorSetLayout)
		vkDestroyDescriptorSetLayout(vk_state->device, vk_state->descriptorSetLayout, vk_state->allocator);
}

void UpdateDescriptorSets(u32 index, VulkanImage* image)
{
	VkDescriptorBufferInfo descriptorBufferInfo = {};
	descriptorBufferInfo.buffer = vk_state->uniformBuffersDarray[index];
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(GlobalUniformObject);

	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler = image->sampler;
	descriptorImageInfo.imageView = image->view;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet descriptorWrites[2] = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].pNext = nullptr;
	descriptorWrites[0].dstSet = vk_state->uniformDescriptorSetsDarray[index];
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].pImageInfo = nullptr;
	descriptorWrites[0].pBufferInfo = &descriptorBufferInfo;
	descriptorWrites[0].pTexelBufferView = nullptr;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].pNext = nullptr;
	descriptorWrites[1].dstSet = vk_state->uniformDescriptorSetsDarray[index];
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].pImageInfo = &descriptorImageInfo;
	descriptorWrites[1].pBufferInfo = nullptr;
	descriptorWrites[1].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(vk_state->device, 2, descriptorWrites, 0, nullptr);
}
