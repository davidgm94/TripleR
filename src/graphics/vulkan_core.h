#define NV_RTX 0
#define NV_MESH_SHADING 1
#include <volk.h>
#if NDEBUG
#define VKCHECK(result) (result)
#else
#define VKCHECK(result)\
do {\
	VkResult result___ = result;\
	assert(result___ == VK_SUCCESS);\
} while (0)
#endif


enum Vk_QueueFamilyIndex
{
    VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS,
    VULKAN_QUEUE_FAMILY_INDEX_COMPUTE,
    VULKAN_QUEUE_FAMILY_INDEX_TRANSFER,
};

static inline VkInstance vk_createInstance(VkAllocationCallbacks* allocator)
{
	VKCHECK(volkInitialize());
    VkApplicationInfo applicationInfo;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = "Red Engine";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "Red";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_1;
	    
    u32 instanceLayerCount = 0;
    VKCHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
    assert(instanceLayerCount > 0);

    VkLayerProperties instanceLayers[instanceLayerCount];
    VKCHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers));

    u32 instanceExtensionCount = 0;
    VKCHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));
    assert(instanceExtensionCount > 0);
    VkExtensionProperties extensionProperties[instanceExtensionCount];
    VKCHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, extensionProperties));

	OutputDebugStringA("INSTANCE LAYERS:\n");
	const char* preferredValidationLayer = "VK_LAYER_KHRONOS_validation";
	const char* notPreferredValidationLayer = "VK_LAYER_LUNARG_standard_validation";
	char* validationLayer = null;
	bool preferredValidationLayerPresent = false;
	for (int i = 0; i < instanceLayerCount; i++)
	{
		if (strcmp(instanceLayers[i].layerName, preferredValidationLayer) == 0)
		{
			preferredValidationLayerPresent = true;
		}
		OutputDebugStringA(instanceLayers[i].layerName);
		OutputDebugStringA(": ");
		OutputDebugStringA(instanceLayers[i].description);
		OutputDebugStringA("\n");
	}
	OutputDebugStringA("EXTENSION LAYERS:\n");
	for (int i = 0; i < instanceExtensionCount; i++)
	{
		OutputDebugStringA(extensionProperties[i].extensionName);
		OutputDebugStringA("\n");
	}

	if (preferredValidationLayerPresent)
	{
		validationLayer = (char*)preferredValidationLayer;
	}
	else
	{
		validationLayer = (char*)notPreferredValidationLayer;
	}
	const char* enabledLayers[] =
	{
		validationLayer,
	};

	const char* enabledExtensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
	};

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;
	createInfo.enabledLayerCount = ARRAYCOUNT(enabledLayers);
	createInfo.ppEnabledLayerNames = enabledLayers;
	createInfo.enabledExtensionCount = ARRAYCOUNT(enabledExtensions);
	createInfo.ppEnabledExtensionNames = enabledExtensions;

    VkInstance instance;
	VKCHECK(vkCreateInstance(&createInfo, allocator, &instance));
	volkLoadInstance(instance);

    return instance;
}

#if _DEBUG
VkBool32 vk_debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, u64 object,
    size_t location, int messageCode, const char* pLayerPrefix, const char* pMessage,
    void* pUserData)
{
	object = 5;
	location = 5;
	pUserData = 0;
    // Select prefix depending on flags passed to the callback
		    // Note that multiple flags may be set for a single validation message
    char prefix[20];

    // Error that may result in undefined behaviour
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
	   strcpy(prefix, "ERROR:");
    };
    // Warnings may hint at unexpected / non-spec API usage
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
	   strcpy(prefix, "WARNING:");
    };
    // May indicate sub-optimal usage of the API
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
	   strcpy(prefix, "PERFORMANCE:");
    };
    // Informal messages that may become handy during debugging
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
	   strcpy(prefix, "INFO:");
    }
    // Diagnostic info from the Vulkan loader and layers
    // Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
	   strcpy(prefix, "DEBUG:");
    }

    // Display message to default output (console/logcat)
    char debugMessage[4096];
    os_sprintf(debugMessage, "%s [%s] Code %d:%s\n", prefix, pLayerPrefix, messageCode, pMessage);

	os_printf("%s", debugMessage);
#ifdef _WIN64
    OutputDebugStringA(debugMessage);
#endif

    fflush(stdout);
    // The return value of this callback controls wether the Vulkan call that caused
    // the validation message will be aborted or not
    // We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
    // (and return a VkResult) to abort
    // If you instead want to have calls abort, pass in VK_TRUE and the function will 
    // return VK_ERROR_VALIDATION_FAILED_EXT 
    return VK_FALSE;
}

static inline VkDebugReportCallbackEXT vk_createDebugCallback(VkAllocationCallbacks* allocator, VkInstance instance, VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callback)
{
    VkDebugReportCallbackCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.flags = flags;
    createInfo.pfnCallback = callback;
    createInfo.pUserData = nullptr;

    VkDebugReportCallbackEXT callbackHandle = nullptr;
    VKCHECK(vkCreateDebugReportCallbackEXT(instance, &createInfo, allocator, &callbackHandle));
    return callbackHandle;
}
#endif

static inline u32 vk_getQueueFamilyIndex(VkQueueFlags queueFlags, const VkQueueFamilyProperties* queueFamilyProperties, u32 queueFamilyPropertyArraysize)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
	   for (u32 i = 0; i < queueFamilyPropertyArraysize; i++)
		  if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			 return i;

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlags & VK_QUEUE_TRANSFER_BIT)
	   for (u32 i = 0; i < queueFamilyPropertyArraysize; i++)
		  if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			 return i;

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (u32 i = 0; i < queueFamilyPropertyArraysize; i++)
	   if (queueFamilyProperties[i].queueFlags & queueFlags)
		  return i;

    assert(!"Couldn't find a matching queue");
    return 0;
}

static inline VkBool32 vk_physicalDeviceSupportsPresentation(VkPhysicalDevice physicalDevice, u32 familyIndex)
{
	VkBool32 supportsPresentation = false;
#if VK_USE_PLATFORM_WIN32_KHR
	supportsPresentation = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, familyIndex);
	return supportsPresentation;
#else
	return supportsPresentation;
#endif
}

static inline VkPhysicalDevice vk_pickPhysicalDevice(VkInstance instance)
{
   	u32 physicalDeviceCount = 0;
	VKCHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	VkPhysicalDevice physicalDevices[physicalDeviceCount];

	VKCHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &physicalDevices[0]));
	VkPhysicalDevice physicalDevice = physicalDevices[0];

	// TODO: do picking
	return physicalDevice;
}

static inline VkDeviceQueueCreateInfo getDeviceQueueCreateInfo()
{
	// TODO: ACTUALLY WRITE A PROPER FUNCTION
	float queuePriorities[] = { 0.0f };
	VkDeviceQueueCreateInfo queueInfo;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = null;
    queueInfo.flags = 0;
	queueInfo.queueFamilyIndex = 0;
    queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;
	
	return queueInfo;
}
static inline VkDevice vk_createDevice(VkAllocationCallbacks* allocator, VkPhysicalDevice physicalDevice, VkInstance instance, VkDeviceQueueCreateInfo* queueCreateInfoArray, u32 queueCreateInfoCount)
{
	u32 extensionCount = 0;
	VKCHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
	os_printf("Device extension count: %d\n", extensionCount);
	VkExtensionProperties extensions[extensionCount];
	VKCHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, &extensions[0]));
	os_printf("DEVICE EXTENSIONS\n\n");
	for (int i = 0; i < extensionCount; i++)
	{
		os_printf("%s : %u\n", extensions[i].extensionName, extensions[i].specVersion);
	}

	u32 layerCount = 0;
	VKCHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &layerCount, nullptr));
	os_printf("Device layer count: %d\n", layerCount);
	VkLayerProperties layers[layerCount];
	VKCHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, &layers[0]));
	os_printf("DEVICE LAYERS\n\n");
	for (int i = 0; i < layerCount; i++) 
	{
		os_printf("%s [%u]: %s\n", layers[i].layerName, layers[i].specVersion, layers[i].description);
	}

	const char* enabledExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
		VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
		VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
#if NV_MESH_SHADING
		VK_NV_MESH_SHADER_EXTENSION_NAME,
#endif
	};


	VkPhysicalDeviceFeatures2 features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

	VkPhysicalDevice16BitStorageFeatures features16 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES };
	features16.storageBuffer16BitAccess = true;

	VkPhysicalDevice8BitStorageFeaturesKHR features8 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR };
	features8.storageBuffer8BitAccess = true;
	features8.uniformAndStorageBuffer8BitAccess = true;

	// VkPhysicalDeviceShaderFloat16Int8FeaturesKHR featuresShaderfp16int8 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };
	// featuresfp16int8.shaderInt8 = true;
	VkPhysicalDeviceFloat16Int8FeaturesKHR featuresfp16int8 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };
	featuresfp16int8.shaderInt8 = true;

	
#if NV_MESH_SHADING
	VkPhysicalDeviceMeshShaderFeaturesNV featuresMesh = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV };
	featuresMesh.meshShader = true;
#endif

	VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &features;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfoArray;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = null;
    createInfo.enabledExtensionCount = ARRAYCOUNT(enabledExtensions);
    createInfo.ppEnabledExtensionNames = enabledExtensions;
    createInfo.pEnabledFeatures = null;
	features.pNext = &features16;
	features16.pNext = &features8;
	features8.pNext = &featuresfp16int8;
#if NV_MESH_SHADING
	featuresfp16int8.pNext = &featuresMesh;
	featuresMesh.pNext = null;
#else
	featuresfp16int8.pNext = null;
#endif
	
	VkDevice device;
	VKCHECK(vkCreateDevice(physicalDevice, &createInfo, allocator, &device));

	return device;
}

static inline VkSurfaceKHR vk_createSurface(VkAllocationCallbacks* allocator, VkInstance instance, os_window_handles* window)
{
#if _WIN64
	VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = (HINSTANCE)window->anotherOSHandle;
    createInfo.hwnd = (HWND)window->windowHandle;

	VkSurfaceKHR surface;
	VKCHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, allocator, &surface));

#endif
	return surface;
}
#define QUEUE_FAMILY_PROPERTY_COUNT 34
#define QUEUE_FAMILY_INDEX_COUNT 3
#define SURFACE_FORMAT_COUNT 2
#define PRESENT_MODE_COUNT 3
#define IMAGE_COUNT 3

typedef u32 queue_family_indices[QUEUE_FAMILY_INDEX_COUNT];

typedef struct
{
    VkQueueFamilyProperties properties[QUEUE_FAMILY_PROPERTY_COUNT];
    size_t propertyCount;
    queue_family_indices indices;
} queue_family;

typedef struct
{
    queue_family queueFamily;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR surfaceFormats[SURFACE_FORMAT_COUNT];
	VkPresentModeKHR presentModes[PRESENT_MODE_COUNT];
} swapchain_properties;

static inline void vk_fillSwapchainProperties(swapchain_properties* swapchainProperties, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    u32 queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
    assert(queueFamilyPropertyCount > 0);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, swapchainProperties->queueFamily.properties);
    swapchainProperties->queueFamily.propertyCount = queueFamilyPropertyCount;

    vkGetPhysicalDeviceFeatures(physicalDevice, &swapchainProperties->features);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &swapchainProperties->memoryProperties);
    vkGetPhysicalDeviceProperties(physicalDevice, &swapchainProperties->physicalDeviceProperties);

    VkBool32 surfaceSupportsPresentation;
    VKCHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &surfaceSupportsPresentation));
    assert(surfaceSupportsPresentation);

    VKCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainProperties->surfaceCapabilities));

    u32 surfaceFormatCount = 0;
    VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    assert(surfaceFormatCount > 0);
    //DEBUG_COUNTER("surfaceFormatCount:", surfaceFormatCount);
    VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, swapchainProperties->surfaceFormats));

    u32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    assert(presentModeCount > 0);
    //DEBUG_COUNTER("presentModeCount:", presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapchainProperties->presentModes);
}

static inline VkPresentModeKHR vk_pickPresentMode(const VkPresentModeKHR* presentModes, u32 presentModeCount)
{
    VkPresentModeKHR pickedPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    for (u32 i = 0; i < presentModeCount; i++)
    {
	   VkPresentModeKHR presentMode = presentModes[i];
	   if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
	   {
		  pickedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		  return pickedPresentMode;
	   }
	   if ((pickedPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR))
	   {
		  pickedPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	   }
    }

    return pickedPresentMode;
}

static inline u32 vk_pickImageCount(VkSurfaceCapabilitiesKHR* surfaceCapabilities)
{
    u32 imageCount = surfaceCapabilities->minImageCount + 1;

    if (surfaceCapabilities->maxImageCount > 0 &&
	   imageCount > surfaceCapabilities->maxImageCount)
    {
	   imageCount = surfaceCapabilities->maxImageCount;
    }

    return imageCount;
}

static inline VkExtent2D vk_pickImageExtent(VkSurfaceCapabilitiesKHR* surfaceCapabilities, VkExtent2D* desiredExtent)
{
    VkExtent2D currentExtent = surfaceCapabilities->currentExtent;

    // If the current extent is defined
    if (currentExtent.width != UINT32_MAX)
    {
	   VkExtent2D imageExtent = currentExtent;
	   return imageExtent;
    }
    else
    {
	   VkExtent2D imageExtent;
	   imageExtent.width = desiredExtent->width;
	   imageExtent.height = desiredExtent->height;

	   VkExtent2D minImageExtent = surfaceCapabilities->minImageExtent;
	   VkExtent2D maxImageExtent = surfaceCapabilities->maxImageExtent;

	   if (imageExtent.width < minImageExtent.width)
	   {
		  imageExtent.width = minImageExtent.width;
	   }
	   if (imageExtent.width > maxImageExtent.width)
	   {
		  imageExtent.width = maxImageExtent.width;
	   }
	   if (imageExtent.height < minImageExtent.height)
	   {
		  imageExtent.height = minImageExtent.height;
	   }
	   if (imageExtent.height > maxImageExtent.height)
	   {
		  imageExtent.height = maxImageExtent.height;
	   }

	   return imageExtent;
    }
}

static inline VkImageUsageFlags vk_getImageUsage(VkImageUsageFlags supportedImageUsageFlags, VkImageUsageFlags desiredUsages)
{
    VkImageUsageFlags imageUsage = desiredUsages & supportedImageUsageFlags;
    assert(desiredUsages == imageUsage);
    return imageUsage;
}

static inline VkSurfaceTransformFlagsKHR vk_pickImageTransformation(VkSurfaceCapabilitiesKHR* surfaceCapabilities, VkSurfaceTransformFlagsKHR desiredTransform)
{
    VkSurfaceTransformFlagsKHR surfaceTransform;
    if (surfaceCapabilities->supportedTransforms & desiredTransform)
    {
	   surfaceTransform = desiredTransform;
    }
    else
    {
	   surfaceTransform = surfaceCapabilities->currentTransform;
    }

    return surfaceTransform;
}

typedef struct
{
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D imageExtent;
    VkPresentModeKHR presentMode;
    VkImageUsageFlags imageUsage;
    VkSurfaceTransformFlagsKHR surfaceTransform;
    u32 minImageCount;
}
swapchain_requirements;

static inline VkSurfaceFormatKHR vk_pickSurfaceFormat(const VkSurfaceFormatKHR* surfaceFormats, u32 surfaceFormatCount)
{
    VkSurfaceFormatKHR pickedSurfaceFormat = {0};
    if (surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
	   pickedSurfaceFormat.format = VK_FORMAT_UNDEFINED;
	   pickedSurfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	   return pickedSurfaceFormat;
    }
    else
    {
	   const VkFormat desiredFormat = VK_FORMAT_B8G8R8A8_UNORM;
	   for (u32 i = 0; i < surfaceFormatCount; i++)
	   {
		  VkSurfaceFormatKHR surfaceFormat = surfaceFormats[i];
		  if (surfaceFormat.format == desiredFormat)
		  {
			 pickedSurfaceFormat = surfaceFormat;
			 return pickedSurfaceFormat;
		  }
	   }
    }

    // Simply pick the first one
    pickedSurfaceFormat = surfaceFormats[0];
    return pickedSurfaceFormat;
}

static inline void vk_fillSwapchainRequirements(swapchain_requirements* swapchainRequirements, swapchain_properties* swapchainProperties, VkExtent2D* desiredSwapchainExtent)
{
    swapchainRequirements->surfaceFormat = vk_pickSurfaceFormat(swapchainProperties->surfaceFormats, ARRAYCOUNT(swapchainProperties->surfaceFormats));
    swapchainRequirements->imageExtent = vk_pickImageExtent(&swapchainProperties->surfaceCapabilities, desiredSwapchainExtent);
    swapchainRequirements->presentMode = vk_pickPresentMode(swapchainProperties->presentModes, ARRAYCOUNT(swapchainProperties->presentModes));
    swapchainRequirements->imageUsage = vk_getImageUsage(swapchainProperties->surfaceCapabilities.supportedUsageFlags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    swapchainRequirements->surfaceTransform = vk_pickImageTransformation(&swapchainProperties->surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR); // TODO: investigate further
    swapchainRequirements->minImageCount = vk_pickImageCount(&swapchainProperties->surfaceCapabilities);
}

static inline void vk_getDeviceQueues(VkDevice device, u32* queueFamilyIndices, VkQueue* queues)  
{
	for (int i = 0; i < QUEUE_FAMILY_INDEX_COUNT; i++)
	{
		vkGetDeviceQueue(device, queueFamilyIndices[i], 0, &queues[i]);
	}
}

static inline VkSwapchainKHR vk_createSwapchain(VkAllocationCallbacks* allocator, VkDevice device, VkSurfaceKHR surface, swapchain_requirements* swapchainRequirements, VkSwapchainKHR oldSwapchain)
{
    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = surface;
    createInfo.minImageCount = swapchainRequirements->minImageCount;
    createInfo.imageFormat = swapchainRequirements->surfaceFormat.format;
    createInfo.imageColorSpace = swapchainRequirements->surfaceFormat.colorSpace;
    createInfo.imageExtent = swapchainRequirements->imageExtent;
    createInfo.imageArrayLayers = 1; // TODO: investigate further
    createInfo.imageUsage = swapchainRequirements->imageUsage;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: investigate further
    createInfo.queueFamilyIndexCount = 0; // TODO: investigate further
    createInfo.pQueueFamilyIndices = nullptr; // TODO: investigate further
    createInfo.preTransform = swapchainRequirements->surfaceTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // TODO: investigate further
    createInfo.presentMode = swapchainRequirements->presentMode;
    createInfo.clipped = VK_TRUE; // TODO: investigate further
    createInfo.oldSwapchain = oldSwapchain;
    
    VkSwapchainKHR swapchain;
    VKCHECK(vkCreateSwapchainKHR(device, &createInfo, allocator, &swapchain));

    return swapchain;
}

static inline void vk_getSwapchainImages(VkImage* images, VkDevice device, VkSwapchainKHR swapchain)
{
	u32 imageCount = 0;
	VKCHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
	assert(imageCount > 0);
	VKCHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images));
}

static inline u32 vk_acquireNextImage(VkDevice device, VkSwapchainKHR swapchain, VkSemaphore semaphore)
{
	u32 imageIndex;
	VKCHECK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, nullptr, &imageIndex));
	return imageIndex;
}

static inline VkCommandPool vk_createCommandPool(VkAllocationCallbacks* allocator, VkDevice device, u32 queueFamilyIndex)
{
	// TODO: maybe take a look at these options
	VkCommandPoolCreateFlags commandPoolCreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCommandPoolCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = commandPoolCreateFlags;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	VkCommandPool commandPool;
	VKCHECK(vkCreateCommandPool(device, &createInfo, allocator, &commandPool));

	return commandPool;
}

static inline void vk_createCommandPools(VkAllocationCallbacks* allocator, VkDevice device, VkCommandPool* commandPools, u32* queueFamilyIndices)
{
	for (int i = 0; i < QUEUE_FAMILY_INDEX_COUNT; i++)
	{
		commandPools[i] = vk_createCommandPool(allocator, device, queueFamilyIndices[i]);
	}
}

static inline void vk_createCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 commandBufferCount, VkCommandBuffer* commandBufferArray)
{
	// TODO: maybe take a look at this
	VkCommandBufferLevel cmdBufferLvl = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = cmdBufferLvl;
	commandBufferAllocateInfo.commandBufferCount = commandBufferCount;

	VkCommandBuffer commandBuffersLocal[commandBufferCount];
	VKCHECK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffersLocal));
	os_memcpy(commandBufferArray, commandBuffersLocal, sizeof(VkCommandBuffer) * commandBufferCount);
}

static inline void vk_beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags commandBufferUsage, VkRenderPass renderPass, u32 subpass, VkFramebuffer framebuffer, VkQueryPipelineStatisticFlags pipelineStatistics)
{
	VkCommandBufferInheritanceInfo cbInheritanceInfo;
	cbInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cbInheritanceInfo.pNext = nullptr;
	cbInheritanceInfo.renderPass = renderPass;		// can be nullptr
	cbInheritanceInfo.subpass = subpass;			// ignored if renderPass == nullptr
	cbInheritanceInfo.framebuffer = framebuffer;	// can be nullptr
	cbInheritanceInfo.occlusionQueryEnable = false; // TODO: check this?
	cbInheritanceInfo.queryFlags = 0;				// TODO: maybe error?
	cbInheritanceInfo.pipelineStatistics = pipelineStatistics;

	VkCommandBufferBeginInfo cbBeginInfo;
	cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbBeginInfo.pNext = nullptr;
	cbBeginInfo.flags = commandBufferUsage;
	cbBeginInfo.pInheritanceInfo = &cbInheritanceInfo;

	VKCHECK(vkBeginCommandBuffer(commandBuffer, &cbBeginInfo));
}

static inline void vk_beginOneTimeSubmitCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkCommandBufferBeginInfo cbBeginInfo;
	cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbBeginInfo.pNext = nullptr;
	cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbBeginInfo.pInheritanceInfo = null;

	VKCHECK(vkBeginCommandBuffer(commandBuffer, &cbBeginInfo));
}

static inline void vk_endCommandBuffer(VkCommandBuffer commandBuffer)
{
	VKCHECK(vkEndCommandBuffer(commandBuffer));
}

static inline void vk_resetCommandBuffer(VkCommandBuffer commandBuffer)
{
	// TODO: actually not releasing the memory back to the command pool
	VkCommandBufferResetFlags resetFlags = 0;
	VKCHECK(vkResetCommandBuffer(commandBuffer, resetFlags));
}

static inline void vk_resetCommandPool(VkCommandPool commandPool, VkDevice device)
{
	// TODO: actually not releasing the memory 
	VkCommandPoolResetFlags resetFlags = 0;
	VKCHECK(vkResetCommandPool(device, commandPool, resetFlags));
}

static inline VkSemaphore vk_createSemaphore(VkAllocationCallbacks* allocator, VkDevice device)
{
	VkSemaphoreCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.pNext = nullptr;
	VkSemaphore semaphore;
	VKCHECK(vkCreateSemaphore(device, &createInfo, allocator, &semaphore));

	return semaphore;
}

static inline VkFence vk_createFence(VkAllocationCallbacks* allocator, VkDevice device, VkFenceCreateFlagBits syncState)
{
	VkFenceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = syncState;
	VkFence fence;
	VKCHECK(vkCreateFence(device, &createInfo, allocator, &fence));

	return fence;
}

static inline void vk_waitForFences(VkDevice device, VkFence* fenceArray, u32 fenceCount, u64 timeout, VkBool32 waitForAll)
{
	// TODO: better handle return value	(VK_TIMEOUT in case of error)
	VKCHECK(vkWaitForFences(device, fenceCount, fenceArray, waitForAll, timeout));
}

// Semaphores automatically are reset. Fences are not
static inline void vk_resetFences(VkDevice device, VkFence* fenceArray, u32 fenceCount)
{
	VKCHECK(vkResetFences(device, fenceCount, fenceArray));
}

static inline VkImageMemoryBarrier vk_createImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier imageBarrier;
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.pNext = null;
	imageBarrier.srcAccessMask = srcAccessMask;
    imageBarrier.dstAccessMask = dstAccessMask;
    imageBarrier.oldLayout = oldLayout;
    imageBarrier.newLayout = newLayout;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // TODO: change
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // TODO: change
	imageBarrier.image = image;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: change
    imageBarrier.subresourceRange.baseMipLevel = 0; // TODO: change
    imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS; // TODO: change
    imageBarrier.subresourceRange.baseArrayLayer = 0; // TODO: change
    imageBarrier.subresourceRange.layerCount = 1; // TODO: change

	return imageBarrier;
}

static inline void vk_createImageMemoryBarriers(VkImageMemoryBarrier* barriers, VkImage* images, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		barriers[i] = vk_createImageMemoryBarrier(images[i], srcAccessMask, dstAccessMask, oldLayout, newLayout);
	}
}

// WARNING: Make sure that none of these command buffers is currently processed by the device, or were recorded with a VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT flag.
static inline void vk_submitCommandsToQueue(VkSemaphore* waitSemaphoreArray, u32 waitSemaphoreCount, VkPipelineStageFlags* pipelineStageArray,
	VkQueue queue, VkCommandBuffer* commandBufferArray, u32 commandBufferCount,
	VkSemaphore* signalSemaphoreArray, u32 signalSemaphoreCount, VkFence fence) // fence can be nullptr
{
	// TODO: I have based all the computation in the premise that the element count of all elements
	// are the same, stored in the variable semaphoreCount. This may be wrong. Check.
	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = waitSemaphoreCount;
	submitInfo.pWaitSemaphores = waitSemaphoreArray;
	submitInfo.pWaitDstStageMask = pipelineStageArray;
	submitInfo.commandBufferCount = commandBufferCount;
	submitInfo.pCommandBuffers = commandBufferArray;
	submitInfo.signalSemaphoreCount = signalSemaphoreCount;
	submitInfo.pSignalSemaphores = signalSemaphoreArray;

	// TODO: Maybe handle the magical constant submitCount to do a more asynchronous work
#define SUBMITCOUNT 1
	VKCHECK(vkQueueSubmit(queue, SUBMITCOUNT, &submitInfo, fence));
}

// In line with TODO comments in previous function, we should batch command submission to queues: HOW TO?

//	We shouldn't submit command buffers if they were already submitted and their execution
//	hasn't ended yet. We can do this only when command buffers were recorded with a
//	VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT flag, but we should avoid using
//	this flag for performance reasons.
static inline void vk_synchronizeTwoCommandBuffers(VkSemaphore* semaphoreArray1, u32 semaphoreArray1Count, VkPipelineStageFlags* pipelineStageArray1,
	VkQueue queue1, VkCommandBuffer* commandBufferArray1, u32 commandBufferCount1,
	VkSemaphore* synchronizingSemaphoreArray, u32 synchronizingSemaphoreCount, VkPipelineStageFlags* synchronizingPipelineStageArray,
	VkQueue queue2, VkCommandBuffer* commandBufferArray2, u32 commandBufferArray2Count,
	VkSemaphore* signalSemaphoreArray, u32 signalSemaphoreCount, VkFence fence)
{

	// TODO: maybe these three lines are unnecessary? Remove if so.
	u32 firstSignalSemaphoreCount = synchronizingSemaphoreCount;
	VkSemaphore firstSignalSemaphoreArray[firstSignalSemaphoreCount];
	os_memcpy(firstSignalSemaphoreArray, synchronizingSemaphoreArray, sizeof(VkSemaphore) * firstSignalSemaphoreCount);

	vk_submitCommandsToQueue(semaphoreArray1, semaphoreArray1Count, pipelineStageArray1,
		queue1, commandBufferArray1, commandBufferCount1,
		firstSignalSemaphoreArray, firstSignalSemaphoreCount, VK_NULL_HANDLE);

	vk_submitCommandsToQueue(synchronizingSemaphoreArray, synchronizingSemaphoreCount, synchronizingPipelineStageArray,
		queue2, commandBufferArray2, commandBufferArray2Count,
		signalSemaphoreArray, signalSemaphoreCount, fence);
}

static inline void vk_waitForCommands(VkDevice device,
	VkSemaphore* waitSemaphoreArray, u32 waitSemaphoreCount, VkPipelineStageFlags* pipelineStageArray,
	VkQueue queue, VkCommandBuffer* commandBufferArray, u32 commandBufferCount,
	VkSemaphore* signalSemaphoreArray, u32 signalSemaphoreCount,
	VkFence fence, u64 timeout)
{
	vk_submitCommandsToQueue(waitSemaphoreArray, waitSemaphoreCount, pipelineStageArray,
		queue, commandBufferArray, commandBufferCount,
		signalSemaphoreArray, signalSemaphoreCount, fence);

	// don't wait for all
	vk_waitForFences(device, &fence, 1, timeout, false);
}

// TODO: change return value if needed
static inline void waitForQueue(VkQueue queue)
{
	VKCHECK(vkQueueWaitIdle(queue));
}

static inline void waitForDevice(VkDevice device)
{
	VKCHECK(vkDeviceWaitIdle(device));
}

// TODO: change it to work with different queues
static inline VkBuffer vk_createBuffer(VkAllocationCallbacks* allocator, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage)
{
	VkBufferCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0; // TODO: take a look at this!
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: change in the future to support multiple queues!
	createInfo.queueFamilyIndexCount = 0; // TODO """
	createInfo.pQueueFamilyIndices = nullptr; // TODO """

	VkBuffer buffer;
	VKCHECK(vkCreateBuffer(device, &createInfo, allocator, &buffer));

	return buffer;
}

static inline u32 vk_findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDeviceMemoryProperties* memoryProperties)
{
	u32 memoryTypeCount = memoryProperties->memoryTypeCount;
	for (u32 i = 0; i < memoryTypeCount; i++)
		if (typeFilter & (1 << i) && (memoryProperties->memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	assert(!"Error");
	return ~0u;
}

static inline VkDeviceMemory vk_allocateAndBindToBuffer(VkAllocationCallbacks* allocator, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags memoryProperties, VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties)
{
	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &bufferMemoryRequirements);

	VkDeviceMemory bufferMemory;

	u32 memoryTypeIndex = vk_findMemoryType(bufferMemoryRequirements.memoryTypeBits, memoryProperties, physicalDeviceMemoryProperties);

	VkMemoryAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.allocationSize = bufferMemoryRequirements.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	// TODO: maybe allocate a unique big chunk of memory instead of a piece every time we create a buffer
	VKCHECK(vkAllocateMemory(device, &allocateInfo, allocator, &bufferMemory));

	VKCHECK(vkBindBufferMemory(device, buffer, bufferMemory, 0));

	return bufferMemory;
}

static inline void vk_bufferMemoryBarrier(
	VkBuffer* bufferArray, u32 bufferCount,
	VkAccessFlags* currentAccessFlagArray, VkAccessFlags* newAccessFlagArray,
	u32* currentQueueFamilyArray, u32* newQueueFamilyArray,

	VkCommandBuffer commandBuffer, VkPipelineStageFlags generatingStages, VkPipelineStageFlags consumingStages)
{
	VkBufferMemoryBarrier memoryBarrierArray[bufferCount];

	for (u32 i = 0; i < bufferCount; i++)
	{
		memoryBarrierArray[i] = (VkBufferMemoryBarrier)
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			nullptr,
			currentAccessFlagArray[i],
			newAccessFlagArray[i],
			currentQueueFamilyArray[i],
			newQueueFamilyArray[i],
			bufferArray[i],
			0,
			VK_WHOLE_SIZE
		};
	}

	// TODO: check buffer memory barrier array count
	vkCmdPipelineBarrier(commandBuffer, generatingStages, consumingStages, 0, 0, nullptr, bufferCount, memoryBarrierArray, 0, nullptr);
}

static inline VkBufferView vk_createBufferView(VkAllocationCallbacks* allocator, VkDevice device, VkBuffer buffer, VkFormat bufferFormat, VkDeviceSize memoryOffset, VkDeviceSize memoryRange)
{
	VkBufferViewCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0; // TODO: take a look at this
	createInfo.buffer = buffer;
	createInfo.format = bufferFormat;
	createInfo.offset = memoryOffset;
	createInfo.range = memoryRange;
	
	VkBufferView bufferView;
	VKCHECK(vkCreateBufferView(device, &createInfo, allocator, &bufferView));

	return bufferView;
}

typedef enum
{
	RENDER_CUBE_FALSE,
	RENDER_CUBE_TRUE,
} RenderCube;

static inline VkImage vk_createImage(VkAllocationCallbacks* allocator, VkDevice device,
	VkImageType type, VkFormat format, VkExtent3D* extent, u32 mipmapLevelCount, u32 layerCount, VkSampleCountFlagBits samples, VkImageUsageFlags usage, RenderCube renderCube)
{
	VkImageCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = renderCube == RENDER_CUBE_TRUE ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0; // TODO: look at this in the future
	createInfo.imageType = type;
	createInfo.format = format;
	createInfo.extent = *extent;
	createInfo.mipLevels = mipmapLevelCount;
	createInfo.arrayLayers = layerCount;
	createInfo.samples = samples;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // TODO ????? is this all we want? PS: optimal is the best option for performance (but what if we want to use linear for some reason?)
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	// TODO: obviously take a look at this into the future to support multiple queue family indices
	createInfo.queueFamilyIndexCount = 0;				// TODO: obviously take a look at this into the future to support multiple queue family indices
	createInfo.pQueueFamilyIndices = nullptr;			// TODO: obviously take a look at this into the future to support multiple queue family indices
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // ?? TODO: maybe this can be a hit on performance?

	VkImage image;
	VKCHECK(vkCreateImage(device, &createInfo, allocator, &image));

	return image;
}


// TODO:
/*
	Similarly to binding memory objects to buffers, we should allocate bigger memory objects
	and bind parts of them to multiple images.This way, we perform fewer memory allocations
	and the driver has to track a smaller number of memory objects.This may improve the
	performance of our application.It may also allow us to save some memory, as each
	allocation may require more memory than requested during allocation(in other words, its
	size may always be rounded up to a multiple of the memory page size).Allocating bigger
	memory objects and reusing parts of them for multiple images spares us the wasted area.
*/
static inline VkDeviceMemory vk_allocateAndBindToImage(VkAllocationCallbacks* allocator, VkDevice device, VkImage image,
	VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties)
{
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkDeviceMemory imageMemory;
	u32 memoryTypeIndex = vk_findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags, physicalDeviceMemoryProperties);

	VkMemoryAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VKCHECK(vkAllocateMemory(device, &allocateInfo, allocator, &imageMemory));
	VKCHECK(vkBindImageMemory(device, image, imageMemory, 0));

	return imageMemory;
}

static inline void vk_imageMemoryBarrier( /* { */
	VkImage* imageArray, u32 imageCount,
	VkAccessFlags* currentAccessFlagArray, VkAccessFlags* newAccessFlagArray,
	VkImageLayout* currentLayoutArray, VkImageLayout* newLayoutArray,
	u32* currentQueueFamilyArray, u32* newQueueFamilyArray,
	VkImageAspectFlags* aspectArray,
	/* } */
	VkCommandBuffer commandBuffer, VkPipelineStageFlags generatingStages, VkPipelineStageFlags consumingStages)
{
	VkImageMemoryBarrier memoryBarrierArray[imageCount];

	for (u32 i = 0; i < imageCount; i++)
	{
		memoryBarrierArray[i] = (VkImageMemoryBarrier)
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			currentAccessFlagArray[i],
			newAccessFlagArray[i],
			currentLayoutArray[i],
			newLayoutArray[i],
			currentQueueFamilyArray[i],
			newQueueFamilyArray[i],
			imageArray[i],
		(VkImageSubresourceRange){aspectArray[i], 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS }// TODO:  modify???
		};
	}

	// TODO: check image memory barrier array count
	vkCmdPipelineBarrier(commandBuffer, generatingStages, consumingStages, 0, 0, nullptr, 0, nullptr, imageCount, memoryBarrierArray);
}

static inline VkImageView vk_createImageView(VkAllocationCallbacks* allocator, VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect)
{
	VkImageViewCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0; // TODO: modify into the future?
	createInfo.image = image;
	createInfo.viewType = type;
	createInfo.format = format;
	// todo: take a look at these two last options
	createInfo.components = (VkComponentMapping){ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	createInfo.subresourceRange = (VkImageSubresourceRange){ aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };

	VkImageView imageView;
	VKCHECK(vkCreateImageView(device, &createInfo, allocator, &imageView));

	return imageView;
}

static inline void vk_createImageViews(VkAllocationCallbacks* allocator, VkDevice device, VkImage* images, VkImageView* imageViews, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect)
{
	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		imageViews[i] = vk_createImageView(allocator, device, images[i], type, format, aspect);
	}
}


typedef struct
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
} VulkanImage;

static inline VulkanImage vk_create2DImage(VkAllocationCallbacks* allocator, VkDevice device, VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	VkFormat format, VkExtent2D* extent2D, u32 mipmapLevelCount, u32 layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage, VkImageAspectFlags aspect, RenderCube renderCube)
{
	VkExtent3D extent = (VkExtent3D){ extent2D->width, extent2D->height, 1 };
	VkImage image = vk_createImage(allocator, device, VK_IMAGE_TYPE_2D, format, &extent, mipmapLevelCount, layerCount, sampleCount, usage, renderCube);
	VkDeviceMemory memory = vk_allocateAndBindToImage(allocator, device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	VkImageView imageView = vk_createImageView(allocator, device, image, VK_IMAGE_VIEW_TYPE_2D, format, aspect);

	return (VulkanImage) { image, memory, imageView };
}

static inline VulkanImage vk_createLayered2DImageWithCubemapView(VkAllocationCallbacks* allocator, VkDevice device, VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	u32 sideExtent, u32 mipmapLevelCount, VkImageUsageFlags usage, VkImageAspectFlags aspect)
{
	VkExtent3D extent = (VkExtent3D){ sideExtent, sideExtent, 1 };
	VkImage image = vk_createImage(allocator, device, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, &extent, mipmapLevelCount, 6, VK_SAMPLE_COUNT_1_BIT, usage, true);
	VkDeviceMemory memory = vk_allocateAndBindToImage(allocator, device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	VkImageView imageView = vk_createImageView(allocator, device, image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, aspect);

	return (VulkanImage) { image, memory, imageView };
}

static inline void vk_mapAndCopyMemoryToDevice(VkDevice device, VkDeviceMemory hostVisibleMemory, VkDeviceSize offset, VkDeviceSize dataSize, void* data, bool unmap, void** pointerToDeviceLocalMemory)
{
	void* pointerToMemory;
	// VkMemoryMapFlags memoryMapFlags = 0; // TODO: take a look at this in the future
	VKCHECK(vkMapMemory(device, hostVisibleMemory, offset, dataSize, 0, &pointerToMemory));
	os_memcpy(pointerToMemory, data, dataSize);

	VkMappedMemoryRange memoryRanges[] =
	{
		{
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			hostVisibleMemory,
			offset,
			VK_WHOLE_SIZE
		}
	};

	VKCHECK(vkFlushMappedMemoryRanges(device, ARRAYCOUNT(memoryRanges), memoryRanges));

	if (unmap)
	{
		vkUnmapMemory(device, hostVisibleMemory);
	}
	else if (pointerToDeviceLocalMemory != nullptr)
	{
		*pointerToDeviceLocalMemory = pointerToMemory;
	}
}

// WARNING: Command buffer must be in recording state.
static inline void vk_copyDataBetweenBuffers(VkCommandBuffer commandBuffer,
	VkBuffer dstBuffer, VkBuffer srcBuffer,
	u32 bufferRegionCount, VkBufferCopy* bufferRegionArray)
{
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, bufferRegionCount, bufferRegionArray);
}

// WARNING: Command buffer must be in recording state.
static inline void vk_copyDataFromBufferToImage(VkCommandBuffer commandBuffer,
	VkBuffer buffer, VkImage image,	VkImageLayout currentImageLayout,
	u32 bufferRegionCount, VkBufferImageCopy* bufferRegionArray)
{
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, currentImageLayout, bufferRegionCount, bufferRegionArray);
}

// WARNING: Command buffer must be in recording state.
static inline void vk_copyDataFromImageToBuffer(VkCommandBuffer commandBuffer,
	VkImage image, VkImageLayout currentImageLayout, VkBuffer buffer,	
	u32 copyRegionCount, VkBufferImageCopy* regionArray)
{
	vkCmdCopyImageToBuffer(commandBuffer, image, currentImageLayout, buffer, copyRegionCount, regionArray);
}

/*
	In real-life scenarios, we should use an existing buffer and reuse it as a staging buffer as
	many times as possible to avoid unnecessary buffer creation and destruction operations.
	This way, we also avoid waiting on a fence.
*/	

// BIG TODO: CLEANUP POINTER CASTING
static inline void vk_updateDeviceLocalMemoryBufferThroughStagingBuffer(VkAllocationCallbacks* allocator, VkDevice device, VkPhysicalDeviceMemoryProperties* memoryProperties,
	VkCommandBuffer commandBuffer, VkQueue queue, VkBuffer dstBuffer,
	VkDeviceSize dstOffset, VkDeviceSize dataSize, void* data,
	VkAccessFlags dstBufferCurrentAccess, VkAccessFlags dstBufferNewAccess,
	VkPipelineStageFlags dstBufferGeneratingStages, VkPipelineStageFlags dstBufferConsumingStages,
	VkSemaphore* signalSemaphoreArray, u32 signalSemaphoreCount)
{
	VkBuffer stagingBuffer = vk_createBuffer(allocator, device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	VkDeviceMemory stagingBufferMemory = vk_allocateAndBindToBuffer(allocator, device, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryProperties);
	vk_mapAndCopyMemoryToDevice(device, stagingBufferMemory, 0, dataSize, data, true, nullptr);
	vk_beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr, 0, nullptr, 0);
	
	vk_bufferMemoryBarrier(&dstBuffer, 1, &dstBufferCurrentAccess,
		&(VkAccessFlags) {VK_ACCESS_TRANSFER_WRITE_BIT}, &(u32) { VK_QUEUE_FAMILY_IGNORED }, &(u32) { VK_QUEUE_FAMILY_IGNORED },
		commandBuffer, dstBufferGeneratingStages, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferCopy bufferCopy = (VkBufferCopy){0, dstOffset, dataSize};
	vk_copyDataBetweenBuffers(commandBuffer, dstBuffer, stagingBuffer, 1, &bufferCopy);

	vk_bufferMemoryBarrier(&dstBuffer, 1,
		&(VkAccessFlags) { VK_ACCESS_TRANSFER_WRITE_BIT }, &dstBufferNewAccess,
		&(u32) { VK_QUEUE_FAMILY_IGNORED }, &(u32) { VK_QUEUE_FAMILY_IGNORED },
		commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, dstBufferConsumingStages);
	
	vk_endCommandBuffer(commandBuffer);

	VkFence fence = vk_createFence(allocator, device, !VK_FENCE_CREATE_SIGNALED_BIT);

	vk_submitCommandsToQueue(nullptr, 0, 0, queue, &(VkCommandBuffer) { commandBuffer }, 1, signalSemaphoreArray, signalSemaphoreCount, fence);

	vk_waitForFences(device, &(VkFence) { fence }, 1, UINT64_MAX, false);

	vkDestroyFence(device, fence, allocator);
	vkFreeMemory(device, stagingBufferMemory, allocator);
	vkDestroyBuffer(device, stagingBuffer, allocator);
}

static inline void vk_updateDeviceLocalMemoryImageThroughStagingBuffer(VkAllocationCallbacks* allocator, VkDevice device, VkPhysicalDeviceMemoryProperties* memoryProperties,
	VkCommandBuffer commandBuffer, VkQueue queue, VkImage dstImage,
	/*VkDevice dstOffset,*/ VkDeviceSize dataSize, void* data,
	VkImageLayout dstImageCurrentLayout, VkImageLayout dstImageNewLayout,
	VkAccessFlags dstImageCurrentAccess, VkAccessFlags dstImageNewAccess,
	VkPipelineStageFlags dstImageGeneratingStages, VkPipelineStageFlags dstImageConsumingStages,
	VkImageAspectFlags dstImageAspect,
	VkImageSubresourceLayers* dstImageSubresource , VkOffset3D* dstImageOffset, VkExtent3D* dstImageExtent,
	VkSemaphore* signalSemaphoreArray, u32 signalSemaphoreCount)
{
	VkBuffer stagingBuffer = vk_createBuffer(allocator, device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	VkDeviceMemory stagingBufferMemory = vk_allocateAndBindToBuffer(allocator, device, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryProperties);
	vk_mapAndCopyMemoryToDevice(device, stagingBufferMemory, 0, dataSize, data, true, nullptr);
	vk_beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr, 0, nullptr, 0);

	vk_imageMemoryBarrier(&dstImage, 1,
		&dstImageCurrentAccess, &(VkAccessFlags) { VK_ACCESS_TRANSFER_WRITE_BIT },
		& dstImageCurrentLayout, &(VkImageLayout) { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
		&(u32) { VK_QUEUE_FAMILY_IGNORED }, &(u32) { VK_QUEUE_FAMILY_IGNORED },
		& dstImageAspect,
		commandBuffer,
		dstImageGeneratingStages, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy imageCopy = { 0, 0, 0, *dstImageSubresource, *dstImageOffset, *dstImageExtent };
	vk_copyDataFromBufferToImage(commandBuffer, stagingBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

	vk_imageMemoryBarrier(&dstImage, 1,
		&(VkAccessFlags) { VK_ACCESS_TRANSFER_WRITE_BIT }, &dstImageNewAccess,
		(VkImageLayout*) VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &dstImageNewLayout,
		&(u32) { VK_QUEUE_FAMILY_IGNORED }, &(u32) { VK_QUEUE_FAMILY_IGNORED },
		&dstImageAspect,
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, dstImageConsumingStages);
	
	vk_endCommandBuffer(commandBuffer);

	VkFence fence = vk_createFence(allocator, device, !VK_FENCE_CREATE_SIGNALED_BIT);

	vk_submitCommandsToQueue(nullptr, 0, 0, queue, &(VkCommandBuffer) { commandBuffer }, 1, signalSemaphoreArray, signalSemaphoreCount, fence);

	vk_waitForFences(device, &(VkFence) { fence }, 1, UINT64_MAX, false);

	vkDestroyFence(device, fence, allocator);
	vkFreeMemory(device, stagingBufferMemory, allocator);
	vkDestroyBuffer(device, stagingBuffer, allocator);
}

typedef enum
{
	ANISOTROPY_DISABLED,
	ANISOTROPY_ENABLED,
} anisotropy_config;

typedef enum
{
	COMPARE_OP_DISABLED,
	COMPARE_OP_ENABLED,
} compare_op_config;

typedef enum
{
	UNNORMALIZED_COORDINATES_DISABLED,
	UNNORMALIZED_COORDINATES_ENABLED,
} unnormalized_coordinates_config;
/*
	Descriptors are opaque data structures that represent shader resources.
	They are organized
	into groups or sets and their contents are specified by descriptor set layouts. To provide
	resources to shaders, we bind descriptor sets to pipelines. We can bind multiple sets at once.
	To access resources from within shaders, we need to specify from which set and from which
	location within a set (called a binding) the given resource is acquired.
*/

static inline VkSampler vk_createSampler(VkAllocationCallbacks* allocator, VkDevice device,
	VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
	VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w,
	float miploadBias, anisotropy_config anisotropyEnable, float maxAnisotropy, 
	compare_op_config compareEnable, VkCompareOp compareOp,
	float minLOD, float maxLOD,
	VkBorderColor borderColor, unnormalized_coordinates_config unnormalizedCoordinates)
{
	VkSamplerCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0; // TODO
	createInfo.magFilter = magFilter;
	createInfo.minFilter = minFilter;
	createInfo.mipmapMode = mipmapMode;
	createInfo.addressModeU = u;
	createInfo.addressModeV = v;
	createInfo.addressModeW = w;
	createInfo.mipLodBias = miploadBias;
	createInfo.anisotropyEnable = anisotropyEnable;
	createInfo.maxAnisotropy = maxAnisotropy;
	createInfo.compareEnable = compareEnable;
	createInfo.compareOp = compareOp;
	createInfo.minLod = minLOD;
	createInfo.maxLod = maxLOD;
	createInfo.borderColor = borderColor;
	createInfo.unnormalizedCoordinates = unnormalizedCoordinates;

	VkSampler sampler;
	VKCHECK(vkCreateSampler(device, &createInfo, allocator, &sampler));

	return sampler;
}

static inline VulkanImage vk_createSampledImage(VkAllocationCallbacks* allocator, VkDevice device,
	VkImageType type, VkFormat format, VkExtent3D* extent, u32 mipmapLevelCount, u32 layerCount, VkSampleCountFlagBits samples, VkImageUsageFlags usage, RenderCube renderCube,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	VkImageViewType imageViewType, VkImageAspectFlags aspect)
{
	VkImage image = vk_createImage(allocator, device, type, format, extent, mipmapLevelCount, layerCount, VK_SAMPLE_COUNT_1_BIT, usage | VK_IMAGE_USAGE_SAMPLED_BIT, renderCube);
	VkDeviceMemory memory = vk_allocateAndBindToImage(allocator, device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	VkImageView imageView = vk_createImageView(allocator, device, image, imageViewType, format, aspect);

	return (VulkanImage) { image, memory, imageView };
}

typedef struct
{
	VulkanImage sampledImage;
	VkSampler sampler;
} VulkanCombinedImageSampler;

static inline VulkanCombinedImageSampler vk_createCombinedImageSampler(VkAllocationCallbacks* allocator, VkDevice device,
	VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,
	VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w,
	float miploadBias, anisotropy_config anisotropyEnable, float maxAnisotropy, 
	compare_op_config compareEnable, VkCompareOp compareOp,
	float minLOD, float maxLOD,
	VkBorderColor borderColor, unnormalized_coordinates_config unnormalizedCoordinates,
	VkImageType type, VkFormat format, VkExtent3D* extent, u32 mipmapLevelCount, u32 layerCount, VkSampleCountFlagBits samples, VkImageUsageFlags usage, RenderCube renderCube,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	VkImageViewType imageViewType, VkImageAspectFlags aspect)
{
	VkSampler sampler = vk_createSampler(allocator, device, magFilter, minFilter, mipmapMode, u, v, w, miploadBias, anisotropyEnable, maxAnisotropy, compareEnable, compareOp, minLOD, maxLOD, borderColor, unnormalizedCoordinates);
	VulkanImage sampledImage = vk_createSampledImage(allocator, device, type, format, extent, mipmapLevelCount, layerCount, samples, usage, renderCube, physicalDeviceMemoryProperties, imageViewType, aspect);
	return (VulkanCombinedImageSampler) { sampledImage, sampler };
}

static inline VulkanImage vk_createStorageImage(VkAllocationCallbacks* allocator, VkDevice device,
	VkImageType type, VkFormat format, VkExtent3D* extent, u32 mipmapLevelCount, u32 layerCount, VkImageUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	VkImageViewType imageViewType, VkImageAspectFlagBits aspect)
{
	VkImage image = vk_createImage(allocator, device, type, format, extent, mipmapLevelCount, layerCount, VK_SAMPLE_COUNT_1_BIT, usage | VK_IMAGE_USAGE_STORAGE_BIT, RENDER_CUBE_FALSE);
	VkDeviceMemory memory = vk_allocateAndBindToImage(allocator, device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	VkImageView imageView = vk_createImageView(allocator, device, image, imageViewType, format, aspect);

	return (VulkanImage) { image, memory, imageView };
}

typedef struct
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkBufferView view;
} VulkanBuffer;

static inline VulkanBuffer vk_createUniformTexelBuffer(VkAllocationCallbacks* allocator, VkDevice device,
	VkDeviceSize size, VkBufferUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	VkFormat format)
{
	VkBuffer buffer = vk_createBuffer(allocator, device, size, usage | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
	VkDeviceMemory memory = vk_allocateAndBindToBuffer(allocator, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	VkBufferView view = vk_createBufferView(allocator, device, buffer, format, 0, VK_WHOLE_SIZE); 

	return (VulkanBuffer) { buffer, memory, view };
}

static inline VulkanBuffer vk_createStorageTexelBuffer(VkAllocationCallbacks* allocator, VkDevice device,
	VkDeviceSize size, VkBufferUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	VkFormat format)
{
	VkBuffer buffer = vk_createBuffer(allocator, device, size, usage | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
	VkDeviceMemory memory = vk_allocateAndBindToBuffer(allocator, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	VkBufferView view = vk_createBufferView(allocator, device, buffer, format, 0, VK_WHOLE_SIZE); 

	return (VulkanBuffer) { buffer, memory, view };
}

typedef struct
{
	VkBuffer buffer;
	VkDeviceMemory memory;
} VulkanBufferNoView;

typedef struct
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* data;
	size_t size;
} VulkanBufferWithData;

static inline VulkanBufferWithData vk_createVertexBuffer(VkAllocationCallbacks* allocator, VkDevice device,
	VkDeviceSize size, VkBufferUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties)
{
	VkBuffer buffer = vk_createBuffer(allocator, device, size, usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	VkDeviceMemory memory = vk_allocateAndBindToBuffer(allocator, device, buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physicalDeviceMemoryProperties);

	return (VulkanBufferWithData) { buffer, memory, null, size };
}

static inline VulkanBufferWithData vk_createIndexBuffer(VkAllocationCallbacks* allocator, VkDevice device,
	VkDeviceSize size, VkBufferUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties)
{
	VkBuffer buffer = vk_createBuffer(allocator, device, size, usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	VkDeviceMemory memory = vk_allocateAndBindToBuffer(allocator, device, buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physicalDeviceMemoryProperties);

	return (VulkanBufferWithData) { buffer, memory, null, size };
}

static inline VulkanBufferWithData vk_createStorageBuffer(VkAllocationCallbacks* allocator, VkDevice device,
	VkDeviceSize size, VkBufferUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties)
{
	VkBuffer buffer = vk_createBuffer(allocator, device, size, usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	VkDeviceMemory memory = vk_allocateAndBindToBuffer(allocator, device, buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physicalDeviceMemoryProperties);

	return (VulkanBufferWithData) { buffer, memory, null, size };
}

static inline VulkanBufferNoView vk_createUniformBuffer(VkAllocationCallbacks* allocator, VkDevice device,
	VkDeviceSize size, VkBufferUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties)
{
	VkBuffer buffer = vk_createBuffer(allocator, device, size, usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	VkDeviceMemory memory = vk_allocateAndBindToBuffer(allocator, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);

	return (VulkanBufferNoView) { buffer, memory };
}

// static inline VulkanBufferNoView vk_createStorageBuffer(VkAllocationCallbacks* allocator, VkDevice device,
// 	VkDeviceSize size, VkBufferUsageFlags usage,
// 	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties)
// {
// 	VkBuffer buffer = vk_createBuffer(allocator, device, size, usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
// 	VkDeviceMemory memory = vk_allocateAndBindToBuffer(allocator, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
// 
// 	return (VulkanBufferNoView) { buffer, memory };
// }

static inline VulkanImage vk_createInputAttachment(VkAllocationCallbacks* allocator, VkDevice device,
	VkImageType type, VkFormat format, VkExtent3D* extent, u32 mipmapLevelCount, u32 layerCount, VkImageUsageFlags usage,
	VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties,
	VkImageViewType imageViewType, VkImageAspectFlagBits aspect)
{
	VkImage image = vk_createImage(allocator, device, type, format, extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, RENDER_CUBE_FALSE);
	VkDeviceMemory memory = vk_allocateAndBindToImage(allocator, device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	VkImageView imageView = vk_createImageView(allocator, device, image, imageViewType, format, aspect);

	return (VulkanImage) { image, memory, imageView };
}

static inline VkDescriptorSetLayout vk_createDescriptorSetLayout(VkAllocationCallbacks* allocator, VkDevice device,
	VkDescriptorSetLayoutBinding* bindingArray, u32 bindingCount, VkDescriptorSetLayoutCreateFlags flags)
{
	VkDescriptorSetLayoutCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = null;
    createInfo.flags = flags;
	createInfo.bindingCount = bindingCount;
    createInfo.pBindings = bindingArray;

	VkDescriptorSetLayout setLayout;
	VKCHECK(vkCreateDescriptorSetLayout(device, &createInfo, allocator, &setLayout));

	return setLayout;
}

typedef enum
{
	FREE_DESCRIPTOR_SETS_FALSE, 
	FREE_DESCRIPTOR_SETS_TRUE,
} free_descriptor_sets_config;

static inline VkDescriptorPool vk_createDescriptorPool(VkAllocationCallbacks* allocator, VkDevice device,
	free_descriptor_sets_config freeDescriptorSets,
	u32 maxSetCount, VkDescriptorPoolSize* poolSizeArray, u32 poolSizeCount)
{
	VkDescriptorPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = null;
	createInfo.flags = freeDescriptorSets ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
    createInfo.maxSets = maxSetCount;
    createInfo.poolSizeCount = poolSizeCount;
    createInfo.pPoolSizes = poolSizeArray;

	VkDescriptorPool descriptorPool;
	VKCHECK(vkCreateDescriptorPool(device, &createInfo, allocator, &descriptorPool));

	return descriptorPool;
}

static inline VkDescriptorSet* vk_allocateDescriptorSets(VkAllocationCallbacks* allocator, VkDevice device,
	VkDescriptorPool descriptorPool, VkDescriptorSetLayout* descriptorSetLayouts, u32 descriptorSetLayoutCount,
	VkDescriptorSet* descriptorSets)
{
	VkDescriptorSetAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = null;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = descriptorSetLayoutCount;
    allocateInfo.pSetLayouts = descriptorSetLayouts;

	VKCHECK(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets));

	return descriptorSets;
}

// TODO: write the function
static inline void vk_updateDescriptorSets()
{
	
}

static inline void vk_bindDescriptorSets(VkCommandBuffer commandBuffer,
	VkPipelineBindPoint pipelineType, VkPipelineLayout graphicsPipelineLayout,
	u32 indexForFirstSet, VkDescriptorSet* descriptorSets, u32 descriptorSetCount,
	u32* dynamicOffsets, u32 dynamicOffsetCount)
{
	vkCmdBindDescriptorSets(commandBuffer, pipelineType, graphicsPipelineLayout, indexForFirstSet, descriptorSetCount, descriptorSets, dynamicOffsetCount, dynamicOffsets);
}

// TODO: WRITE:
/*
- create descriptors with texture and uniform buffer
- free descriptor sets
- reset descriptor pool
- destroy pool/set layout/sampler
*/

/* 6. RENDER PASSES AND FRAMEBUFFERS */

static inline VkRenderPass vk_createRenderPass(VkAllocationCallbacks* allocator, VkDevice device,
	VkAttachmentDescription* attachments, u32 attachmentCount,
	VkSubpassDependency* dependencies, u32 dependencyCount,
	VkSubpassDescription* subpasses, u32 subpassCount)
{
	VkRenderPassCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = null;
    createInfo.flags = 0;
	createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = subpassCount;
    createInfo.pSubpasses = subpasses;
    createInfo.dependencyCount = dependencyCount;
    createInfo.pDependencies = dependencies;

	VkRenderPass renderPass;
	VKCHECK(vkCreateRenderPass(device, &createInfo, allocator, &renderPass));

	return renderPass;
}

static inline VkFramebuffer vk_createFramebuffer(VkAllocationCallbacks* allocator, VkDevice device,
	VkRenderPass renderPass, VkImageView* attachments, u32 attachmentCount,
	VkExtent2D* extent, u32 layers)
{
	u32 width = extent->width;
	u32 height = extent->height;
	VkFramebufferCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = null;
    createInfo.flags = 0;
    //VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR = 0x00000001,
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments = attachments;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = layers;

	VkFramebuffer framebuffer;
	VKCHECK(vkCreateFramebuffer(device, &createInfo, allocator, &framebuffer));

	return framebuffer;
}

static inline void vk_createFramebuffers(VkAllocationCallbacks* allocator, VkDevice device,
	VkFramebuffer* swapchainFramebuffers, VkRenderPass renderPass, VkImageView* attachments, u32 attachmentCount,
	VkExtent2D* extent)
{
	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		swapchainFramebuffers[i] = vk_createFramebuffer(allocator, device, renderPass, &attachments[i], 1, extent, 1);
	}
}

static inline VkRenderPass vk_prepareRenderPassForGeometryRenderingAndPostprocessingSubpasses(VkAllocationCallbacks* allocator, VkDevice device)
{
	VkAttachmentDescription attachments[3];
    attachments[0].flags = 0;
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT,                            
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                 
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;            
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;             
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;           
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                   
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;    
	attachments[1].flags = 0;
	attachments[1].format = VK_FORMAT_D16_UNORM;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[2].flags = 0;
	attachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;
	attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;       
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; 

	VkAttachmentReference depthStencilAttachment;
	depthStencilAttachment.attachment = 1;
	depthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	const u32 firstSubpass = 0;
	const u32 secondSubpass = 1;

	VkSubpassDescription subpassDescriptions[2];
	subpassDescriptions[firstSubpass].flags = 0;
	subpassDescriptions[firstSubpass].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[firstSubpass].inputAttachmentCount = 0;
    subpassDescriptions[firstSubpass].pInputAttachments = null;
    subpassDescriptions[firstSubpass].colorAttachmentCount = 1;
	VkAttachmentReference colorAttachmentReference0;
    colorAttachmentReference0.attachment = 0;
    colorAttachmentReference0.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpassDescriptions[firstSubpass].pColorAttachments = &colorAttachmentReference0;
    subpassDescriptions[firstSubpass].pResolveAttachments = null;
    subpassDescriptions[firstSubpass].pDepthStencilAttachment = &depthStencilAttachment;
    subpassDescriptions[firstSubpass].preserveAttachmentCount = 0;
    subpassDescriptions[firstSubpass].pPreserveAttachments = null;

	subpassDescriptions[secondSubpass].flags = 0;
	subpassDescriptions[secondSubpass].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[secondSubpass].inputAttachmentCount = 1;
	VkAttachmentReference inputAttachmentReference1;	
    inputAttachmentReference1.attachment = 0;
    inputAttachmentReference1.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    subpassDescriptions[secondSubpass].pInputAttachments = &inputAttachmentReference1;
    subpassDescriptions[secondSubpass].colorAttachmentCount = 1;
	VkAttachmentReference colorAttachmentReference1;
    colorAttachmentReference0.attachment = 2;
    colorAttachmentReference0.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpassDescriptions[secondSubpass].pColorAttachments = &colorAttachmentReference1;
    subpassDescriptions[secondSubpass].pResolveAttachments = null;
    subpassDescriptions[secondSubpass].pDepthStencilAttachment = null;
    subpassDescriptions[secondSubpass].preserveAttachmentCount = 0;
    subpassDescriptions[secondSubpass].pPreserveAttachments = null;

	VkSubpassDependency subpassDependency;
	subpassDependency.srcSubpass = 0;
    subpassDependency.dstSubpass = 1;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPass renderPass = vk_createRenderPass(allocator, device, attachments, ARRAYCOUNT(attachments), &subpassDependency, 1, subpassDescriptions, ARRAYCOUNT(subpassDescriptions));
	return renderPass;
}

typedef struct
{
	VulkanImage colorImage;
	VulkanImage depthImage;
	VkRenderPass renderPass;
	VkFramebuffer framebuffer;
} render_pass_setup;

static inline render_pass_setup prepareRenderPassAndFramebufferWithColorAndDepthAttachments(VkAllocationCallbacks* allocator, VkDevice device,
VkPhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties, VkExtent2D* extent)
{
	VulkanImage colorImage = vk_create2DImage(allocator, device, physicalDeviceMemoryProperties, VK_FORMAT_R8G8B8A8_UNORM, extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, RENDER_CUBE_FALSE);
	VulkanImage depthImage = vk_create2DImage(allocator, device, physicalDeviceMemoryProperties, VK_FORMAT_D16_UNORM, extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, RENDER_CUBE_FALSE);

	VkAttachmentDescription attachments[2];
	attachments[0].flags = 0;
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;                   
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[1].flags = 0;
	attachments[1].format = VK_FORMAT_D16_UNORM;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthStencilAttachment;
	depthStencilAttachment.attachment = 1;
	depthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = null;
    subpassDescription.colorAttachmentCount = 1;
	VkAttachmentReference colorAttachmentReference0;
    colorAttachmentReference0.attachment = 0;
    colorAttachmentReference0.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpassDescription.pColorAttachments = &colorAttachmentReference0;
    subpassDescription.pResolveAttachments = null;
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachment;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = null;

	VkSubpassDependency subpassDependency;
	subpassDependency.srcSubpass = 0;
	subpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	subpassDependency.dependencyFlags = 0;

	VkRenderPass renderPass = vk_createRenderPass(allocator, device, attachments, ARRAYCOUNT(attachments), &subpassDependency, 1, &subpassDescription, 1);
	VkImageView imageAttachments[] = { colorImage.view, depthImage.view };
	VkFramebuffer framebuffer = vk_createFramebuffer(allocator, device, renderPass, imageAttachments, ARRAYCOUNT(imageAttachments), extent, 1);

	render_pass_setup renderPassSetup;
	renderPassSetup.colorImage = colorImage;
	renderPassSetup.depthImage = depthImage;
	renderPassSetup.renderPass = renderPass;
	renderPassSetup.framebuffer = framebuffer;

	return renderPassSetup;
}

static inline void vk_beginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, VkClearValue* clearValues, u32 clearValueCount, VkSubpassContents subpassContents)
{
	VkRenderPassBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = null;
    beginInfo.renderPass = renderPass;
    beginInfo.framebuffer = framebuffer;
    beginInfo.renderArea = renderArea;
    beginInfo.clearValueCount = clearValueCount;
    beginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
}

static inline void vk_goToNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents subpasssContents)
{
	vkCmdNextSubpass(commandBuffer, subpasssContents);
}

static inline void vk_endRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
}

static inline VkShaderModule vk_createShaderModule(VkAllocationCallbacks* allocator, VkDevice device, const char* sourceCode, u32 sourceCodeSize)
{
	VkShaderModuleCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = null;
    createInfo.flags = 0;
    createInfo.codeSize = sourceCodeSize;
    createInfo.pCode = (const u32*)sourceCode;

	VkShaderModule shaderModule;
	VKCHECK(vkCreateShaderModule(device, &createInfo, allocator, &shaderModule));

	return shaderModule;
}

static inline VkPipelineLayout vk_createPipelineLayout(VkAllocationCallbacks* allocator, VkDevice device,
	VkDescriptorSetLayout* setLayouts, u32 setLayoutCount,
	VkPushConstantRange* pushConstantRanges, u32 pushConstantRangeCount)
{
	VkPipelineLayoutCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext = null;
    createInfo.flags = 0;
    createInfo.setLayoutCount = setLayoutCount;
    createInfo.pSetLayouts = setLayouts;
    createInfo.pushConstantRangeCount = pushConstantRangeCount;
    createInfo.pPushConstantRanges = pushConstantRanges;

	VkPipelineLayout graphicsPipelineLayout;
	VKCHECK(vkCreatePipelineLayout(device, &createInfo, allocator, &graphicsPipelineLayout));

	return graphicsPipelineLayout;
}

static inline VkPipelineCache vk_createPipelineCache(VkAllocationCallbacks* allocator, VkDevice device,
	u8* cacheData, u32 cacheDataSize)
{
	VkPipelineCacheCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.pNext = null;
    createInfo.flags = 0;
	createInfo.initialDataSize = cacheDataSize;
    createInfo.pInitialData = cacheData;

	VkPipelineCache pipelineCache;
	VKCHECK(vkCreatePipelineCache(device, &createInfo, allocator, &pipelineCache));

	return pipelineCache;
}

static inline u8* vk_retrieveDataFromPipelineCache(VkAllocationCallbacks* allocator, VkDevice device, VkPipelineCache pipelineCache)
{
	size_t dataSize;

	VKCHECK(vkGetPipelineCacheData(device, pipelineCache, &dataSize, null));

	u8* cacheData = null;
	cacheData = malloc(dataSize);

	VKCHECK(vkGetPipelineCacheData(device, pipelineCache, &dataSize, cacheData));

	return cacheData;
}

static inline VkPipelineCache vk_mergePipelineCaches(VkDevice device, VkPipelineCache pipelineCache, const VkPipelineCache* sourceCaches, u32 sourceCacheCount)
{
	VKCHECK(vkMergePipelineCaches(device, pipelineCache, sourceCacheCount, sourceCaches));
	return pipelineCache;
}

static inline VkPipeline* vk_createGraphicsPipelines(VkAllocationCallbacks* allocator, VkDevice device,
	VkPipelineCache pipelineCache, u32 pipelineCount, VkPipeline* pipelines)
{
	VkGraphicsPipelineCreateInfo createInfos[pipelineCount];

	assert(0);
	VKCHECK(vkCreateGraphicsPipelines(device, pipelineCache, pipelineCount, createInfos, allocator, pipelines));
	return pipelines;
}

static inline VkPipeline* vk_createComputePipelines(VkAllocationCallbacks* allocator, VkDevice device,
	VkPipelineCache pipelineCache, u32 pipelineCount, VkPipeline* pipelines)
{
	VkComputePipelineCreateInfo createInfos[pipelineCount];

	assert(0);
	VKCHECK(vkCreateComputePipelines(device, pipelineCache, pipelineCount, createInfos, allocator, pipelines));
	return pipelines;
}

static inline void vk_bindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineType, VkPipeline pipeline)
{
	vkCmdBindPipeline(commandBuffer, pipelineType, pipeline);
}

typedef struct
{
	VkDescriptorSetLayout setLayout;
	VkPipelineLayout graphicsPipelineLayout;
} pipeline;

static inline pipeline vk_createPipelineLayoutWithCombinedImageSamplerBufferAndPushConstantRanges(VkAllocationCallbacks* allocator, VkDevice device, VkPushConstantRange* pushConstantRanges, u32 pushConstantRangeCount)
{
	VkDescriptorSetLayoutBinding bindings[2];
    bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = null;
    bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1].pImmutableSamplers = null;
	
	VkDescriptorSetLayout setLayout = vk_createDescriptorSetLayout(allocator, device, bindings, ARRAYCOUNT(bindings), 0);
	VkPipelineLayout graphicsPipelineLayout = vk_createPipelineLayout(allocator, device, &setLayout, 1, pushConstantRanges, pushConstantRangeCount);

	pipeline pipeline;
	pipeline.graphicsPipelineLayout = graphicsPipelineLayout;
	pipeline.setLayout = setLayout;

	return pipeline;
}

// 21
// 22

// 09 Command Recording and Drawing

static inline void vk_clearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, VkImageSubresourceRange* imageSubresourceRanges, u32 imageSubresourceRangeCount, VkClearColorValue clearColor)
{
	vkCmdClearColorImage(commandBuffer, image, imageLayout, &clearColor, imageSubresourceRangeCount, imageSubresourceRanges);
}

static inline void vk_clearDepthImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, VkImageSubresourceRange* imageSubresourceRanges, u32 imageSubresourceRangeCount, VkClearDepthStencilValue clearColor)
{
	vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, &clearColor, imageSubresourceRangeCount, imageSubresourceRanges);
}

static inline void vk_clearRenderPassAttachments(VkCommandBuffer commandBuffer, VkClearAttachment* attachments, u32 attachmentCount, VkClearRect* rects, u32 rectCount)
{
	vkCmdClearAttachments(commandBuffer, attachmentCount, attachments, rectCount, rects);
}

static inline void vk_bindVertexBuffers(VkCommandBuffer commandBuffer, u32 firstBinding, u32 bindingCount, VkBuffer* buffers, VkDeviceSize* offsets)
{
	vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, buffers, offsets);
}

static inline void vk_bindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize memoryOffset, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(commandBuffer, buffer, memoryOffset, indexType);
}

static inline void vk_sendDataToShader(VkCommandBuffer commandBuffer, VkPipelineLayout graphicsPipelineLayout, VkShaderStageFlags pipelineStages, u32 offset, u32 size, void* data)
{
	vkCmdPushConstants(commandBuffer, graphicsPipelineLayout, pipelineStages, offset, size, data);
}

static inline void vk_setViewportStateDynamically(VkCommandBuffer commandBuffer, u32 firstViewport, VkViewport* viewports, u32 viewportCount)
{
	vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, viewports);
}

static inline void vk_setScissorStateDynamically(VkCommandBuffer commandBuffer, u32 firstScissor, VkRect2D* scissors, u32 scissorCount)
{
	vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, scissors);
}

static inline void vk_setLineWidthStateDynamically(VkCommandBuffer commandBuffer, float lineWidth)
{
	vkCmdSetLineWidth(commandBuffer, lineWidth);
}

static inline void vk_setDepthBiasStateDynamically(VkCommandBuffer commandBuffer, float constantFactor, float clamp, float slopeFactor)
{
	vkCmdSetDepthBias(commandBuffer, constantFactor, clamp, slopeFactor);
}

static inline void vk_setBlendConstantsStateDynamically(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
	vkCmdSetBlendConstants(commandBuffer, blendConstants);
}

static inline void vk_drawGeometry(VkCommandBuffer commandBuffer, u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
{
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

static inline void vk_drawIndexedGeometry(VkCommandBuffer commandBuffer, u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
{
	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

static inline void vk_dispatchComputeWork(VkCommandBuffer commandBuffer, u32 xSize, u32 ySize, u32 zSize)
{
	vkCmdDispatch(commandBuffer, xSize, ySize, zSize);
}

static inline void vk_executeCommandBufferInsideAnother(VkCommandBuffer commandBuffer, VkCommandBuffer* secondaryCommandBuffers, u32 secondaryCBCount)
{
	vkCmdExecuteCommands(commandBuffer, secondaryCBCount, secondaryCommandBuffers);
}



static inline void vk_presentImage(VkQueue graphicsQueue, VkSemaphore* semaphoreArray, u32 semaphoreCount, VkSwapchainKHR* swapchainArray, u32 swapchainCount, u32* imageIndexArray)
{
	VkResult result[swapchainCount];
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = semaphoreCount;
	presentInfo.pWaitSemaphores = semaphoreArray;
	presentInfo.swapchainCount = swapchainCount;
	presentInfo.pSwapchains = swapchainArray;
	presentInfo.pImageIndices = imageIndexArray;
	presentInfo.pResults = result;

	VKCHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));
}


// WARNING: pQueuePriorities might go out of scope!
static inline void vk_setupQueueCreation(VkDeviceQueueCreateInfo* queueCreateInfoArray, queue_family* queueFamily, VkQueueFlags requestedQueueTypes, const float* defaultQueuePriorities)
{
    u32 currentIndex = 0;
    
    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
    {
	   queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS] =
		  vk_getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, queueFamily->properties, queueFamily->propertyCount);
	   VkDeviceQueueCreateInfo queueInfo;
	   queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	   queueInfo.pNext = nullptr;
	   queueInfo.flags = 0;
	   queueInfo.queueFamilyIndex = queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS];
	   queueInfo.queueCount = 1;
	   queueInfo.pQueuePriorities = defaultQueuePriorities;
	   os_memcpy(&queueCreateInfoArray[currentIndex++], &queueInfo, sizeof(queueInfo));
    }
    else
    {
	   queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS] = VK_NULL_HANDLE;
    }

    // Compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
    {
	   queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_COMPUTE] =
		  vk_getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, queueFamily->properties, queueFamily->propertyCount);
	   if (queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_COMPUTE] != queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS])
	   {
		  VkDeviceQueueCreateInfo queueInfo;
		  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		  queueInfo.pNext = nullptr;
		  queueInfo.flags = 0;
		  queueInfo.queueFamilyIndex = queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_COMPUTE];
		  queueInfo.queueCount = 1;
		  queueInfo.pQueuePriorities = defaultQueuePriorities;
		  os_memcpy(&queueCreateInfoArray[currentIndex++], &queueInfo, sizeof(queueInfo));
	   }
    }
    else
    {
	   queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_COMPUTE] = queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS];
    }

    // Transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
    {
	   queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_TRANSFER] =
		  vk_getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, queueFamily->properties, queueFamily->propertyCount);
	   if ((queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_TRANSFER] != queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS])
		  && (queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_TRANSFER] != queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_COMPUTE]))
	   {
		  VkDeviceQueueCreateInfo queueInfo;
		  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		  queueInfo.pNext = nullptr;
		  queueInfo.flags = 0;
		  queueInfo.queueFamilyIndex = queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_TRANSFER];
		  queueInfo.queueCount = 1;
		  queueInfo.pQueuePriorities = defaultQueuePriorities;
		  os_memcpy(&queueCreateInfoArray[currentIndex++], &queueInfo, sizeof(queueInfo));
	   }
    }
    else
    {
	   queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_TRANSFER] = queueFamily->indices[VULKAN_QUEUE_FAMILY_INDEX_GRAPHICS];
    }
}

typedef struct
{
	VkShaderModule vs;
	VkShaderModule fs;
	VulkanBufferWithData vb;
	VulkanBufferWithData ib;
} VulkanTraditionalPipeline;

typedef struct
{
	VkShaderModule vs;
	VkShaderModule ts;
	VkShaderModule ms;
	VkShaderModule fs;
	VulkanBufferWithData vb;
	VulkanBufferWithData ib;
	VulkanBufferWithData mb;
} VulkanMeshPipeline;


// TODO: modify
static inline VkRenderPass vk_setupRenderPass(VkAllocationCallbacks* allocator, VkDevice device,
	VkFormat format)
{
	VkAttachmentDescription attachments[1] = {};
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachments = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass;
	subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = null;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachments;
    subpass.pResolveAttachments = 0;
	subpass.pDepthStencilAttachment = null;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = null;

	VkRenderPass renderPass = vk_createRenderPass(allocator, device, attachments, ARRAYCOUNT(attachments), null, 0, &subpass, 1);

	return renderPass;
}

static inline VkPipeline vk_createGraphicsPipeline(VkAllocationCallbacks* allocator, VkDevice device, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkShaderModule vs, VkShaderModule fs, VkPipelineLayout layout)
{
	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	VkPipelineShaderStageCreateInfo stages[2] = {0};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vs;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fs;
	stages[1].pName = "main";

	createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

	VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	createInfo.pVertexInputState = &vertexInput;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;

	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

	VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	createInfo.pRasterizationState = &rasterizationState;

	VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.pMultisampleState = &multisampleState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	createInfo.pDepthStencilState = &depthStencilState;

	VkPipelineColorBlendAttachmentState colorAttachmentState = {0};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = renderPass;

	VkPipeline pipeline = 0;
	VKCHECK(vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, allocator, &pipeline));

	return pipeline;
}

static inline VkPipeline vk_createMeshGraphicsPipeline(VkAllocationCallbacks* allocator, VkDevice device, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkShaderModule vsOrMs, VkShaderModule fs, VkPipelineLayout layout)
{
	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	VkPipelineShaderStageCreateInfo stages[2] = {0};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

#if NV_MESH_SHADING
	stages[0].stage = VK_SHADER_STAGE_MESH_BIT_NV;
#else
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
#endif

	stages[0].module = vsOrMs;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fs;
	stages[1].pName = "main";

	createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

	VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	createInfo.pVertexInputState = &vertexInput;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;

	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

	VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	createInfo.pRasterizationState = &rasterizationState;

	VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.pMultisampleState = &multisampleState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	createInfo.pDepthStencilState = &depthStencilState;

	VkPipelineColorBlendAttachmentState colorAttachmentState = {0};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = renderPass;

	VkPipeline pipeline = 0;
	VKCHECK(vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, allocator, &pipeline));

	return pipeline;
}

static inline VkQueryPool vk_createTimestampQueryPool(VkAllocationCallbacks* allocator, VkDevice device, u32 queryCount)
{
	VkQueryPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.pNext = null;
    createInfo.flags = 0;
    createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    createInfo.queryCount = queryCount;
	createInfo.pipelineStatistics = 0;

	VkQueryPool queryPool;
	VKCHECK(vkCreateQueryPool(device, &createInfo, allocator, &queryPool));

	return queryPool;

}
