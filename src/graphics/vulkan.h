#pragma once
#include "vulkan_core.h"
typedef struct
{
	// TODO: temporary patch
    VkAllocationCallbacks* allocator;
	VkAllocationCallbacks allocator_;
    VkInstance instance;
#if _DEBUG
    VkDebugReportCallbackEXT debugCallback;
#endif
    swapchain_properties swapchainProperties;
    swapchain_requirements swapchainRequirements;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
	VkQueue queues[QUEUE_FAMILY_INDEX_COUNT];
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
	VkImage swapchainImages[IMAGE_COUNT];
	VkImageView swapchainImageViews[IMAGE_COUNT];
	VkFramebuffer swapchainFramebuffers[IMAGE_COUNT];
	VkImageMemoryBarrier beginRenderBarriers[IMAGE_COUNT];
	VkImageMemoryBarrier endRenderBarriers[IMAGE_COUNT];
	VkQueryPool queryPool;
	VulkanTraditionalPipeline traditionalPipeline;
	VulkanMeshPipeline meshPipeline;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout graphicsPipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPools[QUEUE_FAMILY_INDEX_COUNT];
	VkCommandBuffer commandBuffers[IMAGE_COUNT];
	VkSemaphore imageAcquireSemaphore;
	VkSemaphore imageReleaseSemaphore;
	VkExtent2D extent;
	Mesh_with_meshlets currentMesh;
} vulkan_renderer;

void vk_loadTriangle(vulkan_renderer* vk, os_window_handles* window, os_window_dimensions* windowDimension);
void vk_renderTriangle(vulkan_renderer* vk);
void vk_loadModel(vulkan_renderer* vk, os_window_handles* window, os_window_dimensions* windowDimension);
void vk_renderModel(vulkan_renderer* vk);
void vk_loadModelNVMesh(vulkan_renderer* vk, os_window_handles* window, os_window_dimensions* windowDimension);
void vk_renderModelNVMesh(vulkan_renderer* vk);

static inline void vk_load(vulkan_renderer* vk, os_window_handles* window, os_window_dimensions* windowDimension)
{
	vk_loadModelNVMesh(vk, window, windowDimension);
}

static inline void vk_render(vulkan_renderer* vk)
{
	vk_renderModelNVMesh(vk);
}

void vk_loadModelNVMesh(vulkan_renderer* vk, os_window_handles* window, os_window_dimensions* windowDimension)
{
	//vk->allocator = &vk->allocator_;
	vk->allocator = null;
    // Using volk code to load Vulkan function pointers
    vk->instance = vk_createInstance(vk->allocator);
    // Here ends all Volk code usage
#if _DEBUG
	VkDebugReportFlagsEXT debugCallbackFlags =
		// VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		
    vk->debugCallback = vk_createDebugCallback(vk->allocator, vk->instance, debugCallbackFlags, vk_debugCallback);
#endif
	vk->physicalDevice = vk_pickPhysicalDevice(vk->instance);
	vk->surface = vk_createSurface(vk->allocator, vk->instance, window);
    vk->extent = (VkExtent2D){ windowDimension->width, windowDimension->height };
    vk_fillSwapchainProperties(&vk->swapchainProperties, vk->physicalDevice, vk->surface);
    vk_fillSwapchainRequirements(&vk->swapchainRequirements, &vk->swapchainProperties, &vk->extent);
    VkQueueFlags availableQueues[] = { VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT };
    VkQueueFlags queuesToCreate = 0;
	const float priorities[] = { 0.0f };
    u32 availableQueueCount = ARRAYCOUNT(availableQueues);
    for (u32 i = 0; i < availableQueueCount; i++)
    {
	   queuesToCreate |= availableQueues[i];
    }

	VkDeviceQueueCreateInfo queueCreateInfoArray[ARRAYCOUNT(availableQueues)];
	vk_setupQueueCreation(queueCreateInfoArray, &vk->swapchainProperties.queueFamily, queuesToCreate, priorities);
	vk->device = vk_createDevice(vk->allocator, vk->physicalDevice, vk->instance, queueCreateInfoArray, ARRAYCOUNT(queueCreateInfoArray));
	vk->renderPass = vk_setupRenderPass(vk->allocator, vk->device, vk->swapchainRequirements.surfaceFormat.format);
	vk_getDeviceQueues(vk->device, vk->swapchainProperties.queueFamily.indices, vk->queues);
    vk->swapchain = vk_createSwapchain(vk->allocator, vk->device, vk->surface, &vk->swapchainRequirements, nullptr);
	vk_getSwapchainImages(vk->swapchainImages, vk->device, vk->swapchain);
	vk_createImageViews(vk->allocator, vk->device, vk->swapchainImages, vk->swapchainImageViews, VK_IMAGE_VIEW_TYPE_2D, vk->swapchainRequirements.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
	vk_createFramebuffers(vk->allocator, vk->device, vk->swapchainFramebuffers, vk->renderPass, vk->swapchainImageViews, IMAGE_COUNT, &vk->extent);
	vk_createImageMemoryBarriers(vk->beginRenderBarriers, vk->swapchainImages, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vk_createImageMemoryBarriers(vk->endRenderBarriers, vk->swapchainImages, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	vk->queryPool = vk_createTimestampQueryPool(vk->allocator, vk->device, 128);

	vk->currentMesh = loadMesh("data/kitten.obj");
	raw_str vsFile = os_readFile("src/graphics/shaders/model.vert.spv");
	raw_str fsFile = os_readFile("src/graphics/shaders/model.frag.spv");
	vk->meshPipeline.vs = vk_createShaderModule(vk->allocator, vk->device, vsFile.data, vsFile.size);
	vk->meshPipeline.fs = vk_createShaderModule(vk->allocator, vk->device, fsFile.data, fsFile.size);
#if NV_MESH_SHADING
	raw_str msFile = os_readFile("src/graphics/shaders/model.mesh.spv");
	vk->meshPipeline.ms = vk_createShaderModule(vk->allocator, vk->device, msFile.data, msFile.size);
	os_free(msFile.data);
#endif
	os_free(vsFile.data);
	os_free(fsFile.data);

	VkPipelineCache pipelineCache = null;

#if NV_MESH_SHADING
	VkDescriptorSetLayoutBinding setBindings[2];
	setBindings[0].binding = 0;
	setBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setBindings[0].descriptorCount = 1;
	setBindings[0].stageFlags = VK_SHADER_STAGE_MESH_BIT_NV;
	setBindings[0].pImmutableSamplers = null;

	setBindings[1].binding = 1;
	setBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setBindings[1].descriptorCount = 1;
	setBindings[1].stageFlags = VK_SHADER_STAGE_MESH_BIT_NV;
	setBindings[1].pImmutableSamplers = null;
#else
	VkDescriptorSetLayoutBinding setBindings[1];
	setBindings[0].binding = 0;
	setBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setBindings[0].descriptorCount = 1;
	setBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	setBindings[0].pImmutableSamplers = null;
#endif

	vk->descriptorSetLayout = vk_createDescriptorSetLayout(vk->allocator, vk->device, setBindings, ARRAYCOUNT(setBindings), VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	vk->graphicsPipelineLayout = vk_createPipelineLayout(vk->allocator, vk->device, &vk->descriptorSetLayout, 1, null, 0);
#if NV_MESH_SHADING
	vk->graphicsPipeline = vk_createMeshGraphicsPipeline(vk->allocator, vk->device, pipelineCache, vk->renderPass, vk->meshPipeline.ms, vk->meshPipeline.fs, vk->graphicsPipelineLayout);
#else
	vk->graphicsPipeline = vk_createMeshGraphicsPipeline(vk->allocator, vk->device, pipelineCache, vk->renderPass, vk->meshPipeline.vs, vk->meshPipeline.fs, vk->graphicsPipelineLayout);
#endif

	// COMMANDS AND QUEUES
	vk->imageAcquireSemaphore = vk_createSemaphore(vk->allocator, vk->device);
	vk->imageReleaseSemaphore = vk_createSemaphore(vk->allocator, vk->device);
	vk_createCommandPools(vk->allocator, vk->device, vk->commandPools, vk->swapchainProperties.queueFamily.indices);
	vk_createCommandBuffers(vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], IMAGE_COUNT, vk->commandBuffers);

	vk->currentMesh = loadMesh("data/kitten.obj");

	size_t size = 128 * 1024 * 1024;

	vk->meshPipeline.vb = vk_createVertexBuffer(vk->allocator, vk->device, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &vk->swapchainProperties.memoryProperties);
	VKCHECK(vkMapMemory(vk->device, vk->meshPipeline.vb.memory, 0, vk->meshPipeline.vb.size, 0, &vk->meshPipeline.vb.data));
	os_assert(vk->meshPipeline.vb.size >= vk->currentMesh.vertices.size * sizeof(Vertex));
	os_memcpy(vk->meshPipeline.vb.data, vk->currentMesh.vertices.data, vk->currentMesh.vertices.size * sizeof(Vertex));
	vkUnmapMemory(vk->device, vk->meshPipeline.vb.memory);

	vk->meshPipeline.ib = vk_createIndexBuffer(vk->allocator, vk->device, size, 0, &vk->swapchainProperties.memoryProperties);
	VKCHECK(vkMapMemory(vk->device, vk->meshPipeline.ib.memory, 0, vk->meshPipeline.ib.size, 0, &vk->meshPipeline.ib.data));
	os_assert(vk->meshPipeline.ib.size >= vk->currentMesh.indices.size * sizeof(u32));
	os_memcpy(vk->meshPipeline.ib.data, vk->currentMesh.indices.data, vk->currentMesh.indices.size * sizeof(u32));
	vkUnmapMemory(vk->device, vk->meshPipeline.ib.memory);

#if NV_MESH_SHADING
	vk->meshPipeline.mb = vk_createStorageBuffer(vk->allocator, vk->device, size, 0, &vk->swapchainProperties.memoryProperties);
	VKCHECK(vkMapMemory(vk->device, vk->meshPipeline.mb.memory, 0, vk->meshPipeline.mb.size, 0, &vk->meshPipeline.mb.data));
	os_assert(vk->meshPipeline.mb.size >= vk->currentMesh.meshlets.size * sizeof(Meshlet));
	os_memcpy(vk->meshPipeline.mb.data, vk->currentMesh.meshlets.data, vk->currentMesh.meshlets.size * sizeof(Meshlet));
	vkUnmapMemory(vk->device, vk->meshPipeline.ib.memory);
#endif
}

void vk_renderModelNVMesh(vulkan_renderer* vk)
{
	QPC(startFramePC);
	u32 currentImageIndex = vk_acquireNextImage(vk->device, vk->swapchain, vk->imageAcquireSemaphore);
	VKCHECK(vkResetCommandPool(vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], 0));

	VkCommandBuffer commandBuffer = vk->commandBuffers[currentImageIndex];
	vk_beginOneTimeSubmitCommandBuffer(commandBuffer);

	vkCmdResetQueryPool(commandBuffer, vk->queryPool, 0, 128);
	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vk->queryPool, 0);

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, null, 0, null, 1, &vk->beginRenderBarriers[currentImageIndex]);

	VkClearColorValue color;
	color.float32[0] = 0.0f;
	color.float32[1] = 0.0f;
	color.float32[2] = 0.0f;
	color.float32[3] = 0.0f;
	VkClearValue clearColor = { color };
	
	VkRect2D renderArea;
	renderArea.extent = vk->extent;
	renderArea.offset.x = 0;
	renderArea.offset.y = 0;

	vk_beginRenderPass(commandBuffer, vk->renderPass, vk->swapchainFramebuffers[currentImageIndex], renderArea, &clearColor, 1, VK_SUBPASS_CONTENTS_INLINE);
	
	VkViewport viewport;
    viewport.x = 0;
    viewport.y = (float)vk->extent.height;
    viewport.width = (float)vk->extent.width;
    viewport.height = - (float)vk->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

	VkRect2D scissor = renderArea;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->graphicsPipeline);

	VkDescriptorBufferInfo vbInfo;
    vbInfo.buffer = vk->meshPipeline.vb.buffer;
    vbInfo.offset = 0;
    vbInfo.range = vk->meshPipeline.vb.size;

#if NV_MESH_SHADING
	VkDescriptorBufferInfo mbInfo;
    mbInfo.buffer = vk->meshPipeline.mb.buffer;
    mbInfo.offset = 0;
    mbInfo.range = vk->meshPipeline.mb.size;

	VkWriteDescriptorSet descriptors[2];
	// Vertex buffer
    descriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptors[0].pNext = null;
    descriptors[0].dstSet = null;
    descriptors[0].dstBinding = 0;
    descriptors[0].dstArrayElement = 0;
    descriptors[0].descriptorCount = 1;
    descriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptors[0].pImageInfo = null;
    descriptors[0].pBufferInfo = &vbInfo;
    descriptors[0].pTexelBufferView = null;

    descriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptors[1].pNext = null;
    descriptors[1].dstSet = null;
    descriptors[1].dstBinding = 1;
    descriptors[1].dstArrayElement = 0;
    descriptors[1].descriptorCount = 1;
    descriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptors[1].pImageInfo = null;
    descriptors[1].pBufferInfo = &mbInfo;
    descriptors[1].pTexelBufferView = null;

	vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->graphicsPipelineLayout, 0, ARRAYSIZE(descriptors), descriptors);

	vkCmdDrawMeshTasksNV(commandBuffer, vk->currentMesh.meshlets.size, 0);
#else
	VkWriteDescriptorSet descriptors[1];
    descriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptors[0].pNext = null;
    descriptors[0].dstSet = null;
    descriptors[0].dstBinding = 0;
    descriptors[0].dstArrayElement = 0;
    descriptors[0].descriptorCount = 1;
    descriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptors[0].pImageInfo = null;
    descriptors[0].pBufferInfo = &vbInfo;
    descriptors[0].pTexelBufferView = null;

	vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->graphicsPipelineLayout, 0, ARRAYSIZE(descriptors), descriptors);

	vkCmdBindIndexBuffer(commandBuffer, vk->meshPipeline.ib.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, vk->currentMesh.indices.size, 1, 0, 0, 0);
#endif

	vkCmdEndRenderPass(commandBuffer);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &vk->endRenderBarriers[currentImageIndex]);

	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vk->queryPool, 1);
	vk_endCommandBuffer(commandBuffer);

	VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	vk_submitCommandsToQueue(&vk->imageAcquireSemaphore, 1, &submitStageMask, vk->queues[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], &commandBuffer, 1, &vk->imageReleaseSemaphore, 1, null);

	vk_presentImage(vk->queues[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], &vk->imageReleaseSemaphore, 1, &vk->swapchain, 1, &currentImageIndex);

	VKCHECK(vkDeviceWaitIdle(vk->device));

	u64 queryResults[2];
	VKCHECK(vkGetQueryPoolResults(vk->device, vk->queryPool, 0, ARRAYSIZE(queryResults), sizeof(queryResults), queryResults, sizeof(queryResults[0]), VK_QUERY_RESULT_64_BIT));
	QPC(endFramePC);

	i64 framePC = endFramePC - startFramePC;
	double frameCPU = getTime(framePC) * 1000.0;
	static char buffer[512];

	u64 frameGPUC = queryResults[1] - queryResults[0];
	const float timestampPeriod = vk->swapchainProperties.physicalDeviceProperties.limits.timestampPeriod;
	double frameGPU = frameGPUC * timestampPeriod * 1e-6;
	os_sprintf(buffer, "[Red Engine] Frametime. CPU: %.02f ms. GPU: %.02f ms.", frameCPU, frameGPU);
	SetWindowTextA(win32.handles.window, buffer);
}

#include "vulkan_triangle.h"
#include "vulkan_model.h"
