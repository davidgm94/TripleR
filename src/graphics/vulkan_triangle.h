#pragma once

void vk_loadTriangle(vulkan_renderer* vk, os_window_handles* window, os_window_dimensions* windowDimension)
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

	raw_str vsFile = os_readFile("src/graphics/traditionalPipeline/triangle.vert.spv");
	raw_str fsFile = os_readFile("src/graphics/traditionalPipeline/triangle.frag.spv");
	vk->traditionalPipeline.vs = vk_createShaderModule(vk->allocator, vk->device, vsFile.data, vsFile.size);
	vk->traditionalPipeline.fs = vk_createShaderModule(vk->allocator, vk->device, fsFile.data, fsFile.size);
	free(vsFile.data);
	free(fsFile.data);

	VkPipelineCache pipelineCache = null;

	VkDescriptorSetLayoutBinding setBindings[1];
	setBindings[0].binding = 0;
	setBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setBindings[0].descriptorCount = 1;
	setBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	setBindings[0].pImmutableSamplers = null;

	vk->descriptorSetLayout = vk_createDescriptorSetLayout(vk->allocator, vk->device, setBindings, ARRAYCOUNT(setBindings), VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	vk->graphicsPipelineLayout = vk_createPipelineLayout(vk->allocator, vk->device, &vk->descriptorSetLayout, 1, null, 0);

	vk->graphicsPipeline = vk_createGraphicsPipeline(vk->allocator, vk->device, pipelineCache, vk->renderPass, vk->traditionalPipeline.vs, vk->traditionalPipeline.fs, vk->graphicsPipelineLayout);

	// COMMANDS AND QUEUES
	vk->imageAcquireSemaphore = vk_createSemaphore(vk->allocator, vk->device);
	vk->imageReleaseSemaphore = vk_createSemaphore(vk->allocator, vk->device);
	vk_createCommandPools(vk->allocator, vk->device, vk->commandPools, vk->swapchainProperties.queueFamily.indices);
	vk_createCommandBuffers(vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], IMAGE_COUNT, vk->commandBuffers);
}

void vk_renderTriangle(vulkan_renderer* vk)
{
	u32 currentImageIndex = vk_acquireNextImage(vk->device, vk->swapchain, vk->imageAcquireSemaphore);
	VKCHECK(vkResetCommandPool(vk->device, vk->commandPools[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], 0));

	VkCommandBuffer commandBuffer = vk->commandBuffers[currentImageIndex];
	vk_beginOneTimeSubmitCommandBuffer(commandBuffer);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, null, 0, null, 1, &vk->beginRenderBarriers[currentImageIndex]);

	VkClearColorValue color;
	color.float32[0] = 0.5f;
	color.float32[1] = 0.5f;
	color.float32[2] = 0.5f;
	color.float32[3] = 0.5f;
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

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &vk->endRenderBarriers[currentImageIndex]);

	vk_endCommandBuffer(commandBuffer);

	VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	vk_submitCommandsToQueue(&vk->imageAcquireSemaphore, 1, &submitStageMask, vk->queues[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], &commandBuffer, 1, &vk->imageReleaseSemaphore, 1, null);

	vk_presentImage(vk->queues[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS], &vk->imageReleaseSemaphore, 1, &vk->swapchain, 1, &currentImageIndex);

	VKCHECK(vkDeviceWaitIdle(vk->device));
}
