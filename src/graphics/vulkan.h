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
	VulkanShaderPipeline shaderPipeline;
	VkRenderPass renderPass;
	VulkanPipeline traditionalPipeline;
	VulkanPipeline meshShadingPipeline;
	VkCommandPool commandPools[QUEUE_FAMILY_INDEX_COUNT];
	VkCommandBuffer commandBuffers[IMAGE_COUNT];
	VkSemaphore imageAcquireSemaphore;
	VkSemaphore imageReleaseSemaphore;
	VkExtent2D extent;
	Mesh currentMesh;
    bool meshShadingSupported;
	bool meshShadingEnabled;
} vulkan_renderer;

typedef struct
{
	bool meshShadingSupported;
	bool meshShadingEnabled;
} key_user_data;

void os_handleKeyEvents(void* windowApplication, os_key key, int scancode, int action, int mods)
{
	win32_window_application* winApp = (win32_window_application*)windowApplication;
	key_user_data* keyUserData = (key_user_data*)winApp->keyUserData;

	switch (key)
	{
		case (KEYBOARD_M):
		{
			if (action == (int)os_key_event::PRESS)
			{
				if (keyUserData->meshShadingSupported)
				{
					keyUserData->meshShadingEnabled = !keyUserData->meshShadingEnabled;
				}
			}
		} break;
		default: ;
	}
}

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
	u32 extensionCount = 0;
	VKCHECK(vkEnumerateDeviceExtensionProperties(vk->physicalDevice, 0, &extensionCount, 0));
	vector<VkExtensionProperties> extensions(extensionCount);
	VKCHECK(vkEnumerateDeviceExtensionProperties(vk->physicalDevice, 0, &extensionCount, extensions.data()));

    vk->meshShadingSupported = false;
	vk->meshShadingEnabled = false;
	for (VkExtensionProperties extension : extensions)
    {
	    if (os_strcmp(extension.extensionName, VK_NV_MESH_SHADER_EXTENSION_NAME) == 0)
        {
	        vk->meshShadingSupported = true;
	        break;
        }
    }
	vk->meshShadingEnabled = vk->meshShadingSupported;

	vk->surface = vk_createSurface(vk->allocator, vk->instance, window);
    vk->extent = { windowDimension->width, windowDimension->height };

    VkQueueFlags availableQueues[] = { VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT };
    VkQueueFlags queuesToCreate = 0;
    const float priorities[] = { 0.0f };
    u32 availableQueueCount = ARRAYCOUNT(availableQueues);
    for (u32 i = 0; i < availableQueueCount; i++)
    {
        queuesToCreate |= availableQueues[i];
    }
    vk_fillSwapchainProperties(&vk->swapchainProperties, vk->physicalDevice, vk->surface);
    VkDeviceQueueCreateInfo queueCreateInfoArray[ARRAYCOUNT(availableQueues)];
    vk_setupQueueCreation(queueCreateInfoArray, &vk->swapchainProperties.queueFamily, queuesToCreate, priorities);
    vk_fillSwapchainRequirements(&vk->swapchainRequirements, &vk->swapchainProperties, &vk->extent);

	vk->device = vk_createDevice(vk->allocator, vk->physicalDevice, vk->instance, queueCreateInfoArray, ARRAYCOUNT(queueCreateInfoArray), vk->meshShadingSupported);
	vk->renderPass = vk_setupRenderPass(vk->allocator, vk->device, vk->swapchainRequirements.surfaceFormat.format);
	vk_getDeviceQueues(vk->device, vk->swapchainProperties.queueFamily.indices, vk->queues);
    vk->swapchain = vk_createSwapchain(vk->allocator, vk->device, vk->surface, &vk->swapchainRequirements, nullptr);
	vk_getSwapchainImages(vk->swapchainImages, vk->device, vk->swapchain);
	vk_createImageViews(vk->allocator, vk->device, vk->swapchainImages, vk->swapchainImageViews, VK_IMAGE_VIEW_TYPE_2D, vk->swapchainRequirements.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
	vk_createFramebuffers(vk->allocator, vk->device, vk->swapchainFramebuffers, vk->renderPass, vk->swapchainImageViews, IMAGE_COUNT, &vk->extent);
	vk_createImageMemoryBarriers(vk->beginRenderBarriers, vk->swapchainImages, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vk_createImageMemoryBarriers(vk->endRenderBarriers, vk->swapchainImages, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	vk->queryPool = vk_createTimestampQueryPool(vk->allocator, vk->device, 128);

    char vertexShaderPath[MAX_FILENAME_LENGTH];
    os_getShaderPath("model.vert.spv", vertexShaderPath);
    char fragmentShaderPath[MAX_FILENAME_LENGTH];
    os_getShaderPath("model.frag.spv", fragmentShaderPath);
    raw_str vsFile = os_readFile(vertexShaderPath);
	raw_str fsFile = os_readFile(fragmentShaderPath);
	vk->shaderPipeline.vs = vk_createShaderModule(vk->allocator, vk->device, vsFile.data, vsFile.size);
	vk->shaderPipeline.fs = vk_createShaderModule(vk->allocator, vk->device, fsFile.data, fsFile.size);
    os_free(vsFile.data);
    os_free(fsFile.data);

	if (vk->meshShadingSupported)
    {
        char meshShaderPath[MAX_FILENAME_LENGTH];
        os_getShaderPath("model.mesh.spv", meshShaderPath);
        raw_str msFile = os_readFile(meshShaderPath);
        vk->shaderPipeline.ms = vk_createShaderModule(vk->allocator, vk->device, msFile.data, msFile.size);
        os_free(msFile.data);
    }

	VkPipelineCache pipelineCache = null;

	vector<VkDescriptorSetLayoutBinding> traditionalSetBindings = vk_createDescriptorSetLayoutBindings(false);
    vk->traditionalPipeline.setLayout = vk_createDescriptorSetLayout(vk->allocator, vk->device, traditionalSetBindings.data(), traditionalSetBindings.size(), VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    vk->traditionalPipeline.layout = vk_createPipelineLayout(vk->allocator, vk->device, &vk->traditionalPipeline.setLayout, 1, null, 0);
    vk->traditionalPipeline.pipeline = vk_createGraphicsPipeline(vk->allocator, vk->device, pipelineCache,
                                                                 vk->renderPass, vk->shaderPipeline.vs, vk->shaderPipeline.fs,
                                                                 vk->traditionalPipeline.layout, false);
    if (vk->meshShadingSupported)
    {
        vector<VkDescriptorSetLayoutBinding> meshShadingSetBindings = vk_createDescriptorSetLayoutBindings(true);
        vk->meshShadingPipeline.setLayout = vk_createDescriptorSetLayout(vk->allocator, vk->device, meshShadingSetBindings.data(), meshShadingSetBindings.size(), VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
        vk->meshShadingPipeline.layout = vk_createPipelineLayout(vk->allocator, vk->device, &vk->meshShadingPipeline.setLayout, 1, null, 0);
        vk->meshShadingPipeline.pipeline = vk_createGraphicsPipeline(vk->allocator, vk->device, pipelineCache,
                                                                     vk->renderPass, vk->shaderPipeline.ms, vk->shaderPipeline.fs,
                                                                     vk->meshShadingPipeline.layout, true);
    }

	// COMMANDS AND QUEUES
	vk->imageAcquireSemaphore = vk_createSemaphore(vk->allocator, vk->device);
	vk->imageReleaseSemaphore = vk_createSemaphore(vk->allocator, vk->device);
	vk_createCommandPools(vk->allocator, vk->device, vk->commandPools, vk->swapchainProperties.queueFamily.indices);
	vk_createCommandBuffers(vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], IMAGE_COUNT, vk->commandBuffers);

	char meshPath[MAX_FILENAME_LENGTH];
	os_getAssetPath("kitten.obj", meshPath);
	vk->currentMesh = loadMesh(meshPath, vk->meshShadingSupported);

	const u64 size = 128 * 1024 * 1024;

	vk->shaderPipeline.vb = vk_createVertexBuffer(vk->allocator, vk->device, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &vk->swapchainProperties.memoryProperties);
	os_assert(vk->shaderPipeline.vb.size >= vk->currentMesh.vertices.size() * sizeof(Vertex));

	vk->shaderPipeline.ib = vk_createIndexBuffer(vk->allocator, vk->device, size, 0, &vk->swapchainProperties.memoryProperties);
	os_assert(vk->shaderPipeline.ib.size >= vk->currentMesh.indices.size() * sizeof(u32));

    if (vk->meshShadingSupported)
    {
        vk->shaderPipeline.mb = vk_createMeshBuffer(vk->allocator, vk->device, size, 0,
                                                    &vk->swapchainProperties.memoryProperties);
        os_assert(vk->shaderPipeline.mb.size >= vk->currentMesh.meshlets.size * sizeof(Meshlet));
    }

    vk->shaderPipeline.transferBuffer = vk_createTransferBuffer(vk->allocator, vk->device, size, &vk->swapchainProperties.memoryProperties);
    VKCHECK(vkMapMemory(vk->device, vk->shaderPipeline.transferBuffer.memory, 0, size, 0, &vk->shaderPipeline.transferBuffer.data));

    copyToTransferBuffer(vk->allocator, vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS],
                         vk->commandBuffers[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], vk->queues[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS],
                         &vk->shaderPipeline.vb, &vk->shaderPipeline.transferBuffer, vk->currentMesh.vertices.data(), vk->currentMesh.vertices.size() * sizeof(Vertex));

	copyToTransferBuffer(vk->allocator, vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS],
                         vk->commandBuffers[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], vk->queues[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS],
                         &vk->shaderPipeline.ib, &vk->shaderPipeline.transferBuffer, vk->currentMesh.indices.data(), vk->currentMesh.indices.size() * sizeof(u32));

	if (vk->meshShadingSupported)
	{
        copyToTransferBuffer(vk->allocator, vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS],
                             vk->commandBuffers[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], vk->queues[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS],
                             &vk->shaderPipeline.mb, &vk->shaderPipeline.transferBuffer, vk->currentMesh.meshlets.data(), vk->currentMesh.meshlets.size() * sizeof(Meshlet));
	}

	win32.keyUserData = &vk->meshShadingSupported;
	win32.callbacks.keyEventHandler = os_handleKeyEvents;
}

void vk_renderModelNVMesh(vulkan_renderer* vk)
{
    static double frameAvgCPU = 0;
    static double frameAvgGPU = 0;

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

	VkDescriptorBufferInfo vbInfo;
    vbInfo.buffer = vk->shaderPipeline.vb.buffer;
    vbInfo.offset = 0;
    vbInfo.range = vk->shaderPipeline.vb.size;

    const int drawCount = 1000;
	if (vk->meshShadingEnabled)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->meshShadingPipeline.pipeline);

	    VkDescriptorBufferInfo mbInfo;
        mbInfo.buffer = vk->shaderPipeline.mb.buffer;
        mbInfo.offset = 0;
        mbInfo.range = vk->shaderPipeline.mb.size;

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

	    vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->meshShadingPipeline.layout, 0, ARRAYSIZE(descriptors), descriptors);

	    for (int i = 0; i < drawCount; i++)
	        vkCmdDrawMeshTasksNV(commandBuffer, vk->currentMesh.meshlets.size(), 0);
    }
	else
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->traditionalPipeline.pipeline);

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

        vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->traditionalPipeline.layout, 0, ARRAYSIZE(descriptors), descriptors);

        vkCmdBindIndexBuffer(commandBuffer, vk->shaderPipeline.ib.buffer, 0, VK_INDEX_TYPE_UINT32);
        for (int i = 0; i < drawCount; i++)
            vkCmdDrawIndexed(commandBuffer, vk->currentMesh.indices.size(), 1, 0, 0, 0);
    }

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
	const float timestampHelper = vk->swapchainProperties.physicalDeviceProperties.limits.timestampPeriod;
	static float timestampPeriod = timestampHelper;
	double frameGPU = frameGPUC * timestampPeriod * 1e-6;

	frameAvgCPU = frameAvgCPU * 0.95 + frameCPU * 0.05;
    frameAvgGPU = frameAvgGPU * 0.95 + frameGPU * 0.05;
    static const char * const messageBuffer = "[Red Engine] Frametime. CPU: %.02f ms. GPU: %.02f ms. Triangle count: %u. Meshlet count: %u. Mesh shading: %s";
    static const int triangleCount = int(vk->currentMesh.indices.size() / 3.0);
    static const int meshletCount = int(vk->currentMesh.indices.size());
	os_sprintf(buffer, messageBuffer, frameAvgCPU, frameAvgGPU, triangleCount, meshletCount, vk->meshShadingEnabled ? "ON" : "OFF");
	SetWindowTextA(win32.handles.window, buffer);
}

#include "vulkan_triangle.h"
#include "vulkan_model.h"
