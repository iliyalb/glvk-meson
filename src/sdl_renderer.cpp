#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

bool SdlRenderer::Initialize() {
   	SDL_InitSubSystem(SDL_INIT_VIDEO);
	m_window_ptr = SDL_CreateWindow("Vulkan", m_width, m_height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (!m_window_ptr) {
		showError("[FAILED] window creation unsuccessful");
		return false;
	}

	if (!initializeVulkan()) {
		return false;
	}

	return true;
}

bool SdlRenderer::initializeVulkan() {
	if (!createVulkanInstance()) {
		showError("[FAILED] vulkan instance creation unsuccessful");
		return false;
	}

	if (!createSurface()) {
		showError("[FAILED] window surface creation unsuccessful");
		return false;
	}

	if (m_physicalDevice = findPhysicalDevice(); !m_physicalDevice) {
		showError("[ERROR] instance does not have graphic accelerator");
		return false;
	}

	if (!findGraphicsQueue()) {
		showError("[ERROR] no compatible vulkan capable device found");
		return false;
	}

	if (!createDevice(m_physicalDevice)) {
		showError("[FAILED] logical device creation unsuccessful");
		return false;
	}

	if (!initializeVMA()) {
		showError("[FAILED] vulkan memory allocator failure");
		return false;
	}

	if (!createSwapchain(m_width, m_height)) {
		showError("[FAILED] swapchain creation unsuccessful");
		return false;
	}

	if (!createShaders()) {
		showError("[FAILED] shader module creation unsuccessful");
		return false;
	}

	if (m_pipeline = createGraphicsPipeline(); !m_pipeline) {
		showError("[FAILED] graphics pipeline initialization failure");
		return false;
	}

	if (!createSyncResources()) {
		showError("[FAILED] sync resource failure");
		return false;
	}

	if (!createCommandBuffers()) {
		showError("[FAILED] command buffer object creation unsuccessful");
		return false;
	}

	return true;
}

bool SdlRenderer::createVulkanInstance() {
	if (volkInitialize() != VK_SUCCESS) {
		showError("[ERROR] Volk cannot be initialized");
		return false;
	}

	VkApplicationInfo appInfo {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "My First Triangle",
		.apiVersion = c_vulkanVersion,
	};

	uint32_t m_instExtCount = 0;
	const char *const *c_extensions = SDL_Vulkan_GetInstanceExtensions(&m_instExtCount);

	std::vector<const char *> m_requestedLayers {
		"VK_LAYER_KHRONOS_validation"
	};

	VkInstanceCreateInfo instCreateInfo {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(m_requestedLayers.size()),
		.ppEnabledLayerNames = m_requestedLayers.data(),
		.enabledExtensionCount = m_instExtCount,
		.ppEnabledExtensionNames = c_extensions
	};

	if (vkCreateInstance(&instCreateInfo, nullptr, &m_vulkanInstance) != VK_SUCCESS) {
		return false;
	}

	volkLoadInstance(m_vulkanInstance);
	return true;
}

bool SdlRenderer::createSurface() {
	if (!SDL_Vulkan_CreateSurface(m_window_ptr, m_vulkanInstance, nullptr, &m_surface)) {
		return false;
	}
	return true;
}

VkPhysicalDevice SdlRenderer::findPhysicalDevice() {
	uint32_t m_physDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_vulkanInstance, &m_physDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> m_physicalDevices(m_physDeviceCount);
	vkEnumeratePhysicalDevices(m_vulkanInstance, &m_physDeviceCount, m_physicalDevices.data());

	VkPhysicalDevice m_physicalDevice = nullptr;
	if (m_physDeviceCount) {
		m_physicalDevice = m_physicalDevices[0]; // default GPU
		for (auto &pDev : m_physicalDevices) {
			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(pDev, &props);
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				m_physicalDevice = pDev; // dedicated GPU
				break;
			}
		}
	}

	uint32_t m_formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &m_formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> m_surfaceFormats(m_formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &m_formatCount, m_surfaceFormats.data());

	bool m_formatSupported = false;
	for (const VkSurfaceFormatKHR &c_surfFormat : m_surfaceFormats) {
		if (c_surfFormat.format == c_swapchainFormat) {
			m_formatSupported = true;
			break;
		}
	}
	if (!m_formatSupported) {
		showError("[ERROR] swapchain format not supported by the surface");
		return nullptr;
	}

	return m_physicalDevice;
}

bool SdlRenderer::findGraphicsQueue() {
	uint32_t m_queueFamCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(m_physicalDevice, &m_queueFamCount, nullptr);
	std::vector<VkQueueFamilyProperties2> m_queueFamProps(m_queueFamCount, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
	vkGetPhysicalDeviceQueueFamilyProperties2(m_physicalDevice, &m_queueFamCount, m_queueFamProps.data());

	for (long unsigned int c_index = 0; c_index < m_queueFamProps.size(); c_index++) {
		VkBool32 hasPresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, c_index, m_surface, &hasPresentSupport);

		const auto &c_props = m_queueFamProps[c_index];
		if (c_props.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport) {
			m_gfxQueueFamIdx = c_index;
			return true;
		}
	}
	return false;
}

bool SdlRenderer::createDevice(VkPhysicalDevice p_physicalDevice) {
	float m_queuePriority = 1.0f;
	std::vector<uint32_t> queueFamiles { m_gfxQueueFamIdx };

	VkDeviceQueueCreateInfo gfxQueueInfo {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_gfxQueueFamIdx,
		.queueCount = 1,
		.pQueuePriorities = &m_queuePriority
	};

	VkPhysicalDeviceVulkan14Features m_supportedFeatures14{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = nullptr };
	VkPhysicalDeviceVulkan13Features m_supportedFeatures13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &m_supportedFeatures14 };
	VkPhysicalDeviceVulkan12Features m_supportedFeatures12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &m_supportedFeatures13 };
	VkPhysicalDeviceFeatures2 m_supportedFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &m_supportedFeatures12 };
	vkGetPhysicalDeviceFeatures2(p_physicalDevice, &m_supportedFeatures);

	if (!m_supportedFeatures13.dynamicRendering || !m_supportedFeatures13.synchronization2 ||!m_supportedFeatures12.timelineSemaphore) {
		showError("[FAILED] physical device incompatible");
		return false;
	}

	VkPhysicalDeviceVulkan14Features m_features14 {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
		.pNext = nullptr,
	};

	VkPhysicalDeviceVulkan13Features m_features13 {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &m_features14,
		.synchronization2 = VK_TRUE,
		.dynamicRendering = VK_TRUE,
	};

	VkPhysicalDeviceVulkan12Features m_features12 {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &m_features13,
		.timelineSemaphore = VK_TRUE
	};

	VkPhysicalDeviceFeatures2 m_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &m_features12
	};

	const std::vector<const char *> c_deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo m_devCreateInfo {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &m_features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &gfxQueueInfo,
		.enabledExtensionCount = static_cast<uint32_t>(c_deviceExtensions.size()),
		.ppEnabledExtensionNames = c_deviceExtensions.data(),
		.pEnabledFeatures = nullptr
	};

	if (vkCreateDevice(p_physicalDevice, &m_devCreateInfo, nullptr, &m_device) != VK_SUCCESS) {
		return false;
	}

	vkGetDeviceQueue(m_device, m_gfxQueueFamIdx, 0, &m_gfxQueue);
	if (!m_gfxQueue) {
		showError("[FAILED] graphics queue access failure");
		return false;
	}
	return true;
}

bool SdlRenderer::initializeVMA() {
	VmaVulkanFunctions m_vmaFuncInfo {};
	VmaAllocatorCreateInfo m_vmaAllocInfo {
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = m_physicalDevice,
		.device = m_device,
		.pVulkanFunctions = &m_vmaFuncInfo,
		.instance = m_vulkanInstance,
		.vulkanApiVersion = c_vulkanVersion
	};

	vmaImportVulkanFunctionsFromVolk(&m_vmaAllocInfo, &m_vmaFuncInfo);

	if (vmaCreateAllocator(&m_vmaAllocInfo, &m_vmaAllocator) != VK_SUCCESS) {
		return false;
	}
	return true;
}

bool SdlRenderer::createSwapchain(uint32_t p_width, uint32_t p_height) {
	m_swapchainWidth = p_width;
	m_swapchainHeight = p_height;

	VkSurfaceCapabilitiesKHR m_surfaceCaps {};
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_surfaceCaps) != VK_SUCCESS) {
		showError("[FAILED] surface capabilities unknown");
		return false;
	}

	VkSwapchainCreateInfoKHR m_swapchainCreateInfo {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_surface,
		.minImageCount = m_surfaceCaps.minImageCount,
		.imageFormat = c_swapchainFormat,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent{.width = m_swapchainWidth, .height = m_swapchainHeight },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};

	if (vkCreateSwapchainKHR(m_device, &m_swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
		showError("[FAILED] swapchain creation unsuccessful");
		return false;
	}

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
	m_swapchainImageViews.resize(imageCount);

	for (size_t i = 0; i < m_swapchainImages.size(); ++i) {
		VkImageViewCreateInfo imgViewInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = c_swapchainFormat,
			.subresourceRange {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.levelCount = 1,
				.layerCount = 1
			}
		};

		if (vkCreateImageView(m_device, &imgViewInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) {
			showError("[FAILED] swapchain image view failure");
			return false;
		}
	}

	m_renderCompleteSemaphores.resize(m_swapchainImages.size());
	for (VkSemaphore &semaphore : m_renderCompleteSemaphores) {
		VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
			showError("[FAILED] render completion semaphore failure");
			return false;
		}
	}

	VkImageCreateInfo m_depthCreateInfo {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = c_depthFormat,
		.extent {.width = m_swapchainWidth,.height = m_swapchainHeight, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo m_allocInfo {
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};

	if (vmaCreateImage(m_vmaAllocator, &m_depthCreateInfo, &m_allocInfo, &m_depthImage, &m_depthImageAllocation, nullptr) != VK_SUCCESS) {
		showError("[ERROR] depth image cannot allocate");
		return false;
	}

	VkImageViewCreateInfo depthImgViewInfo {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = m_depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = c_depthFormat,
		.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
	};
	if (vkCreateImageView(m_device, &depthImgViewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
		showError("[ERROR] depth image view cannot be created");
		return false;
	}

	return true;
}

VkShaderModule SdlRenderer::createShaderModule(const std::string &fileName, shaderc_shader_kind kind) const {
	const std::string c_shaderPath = "src/shaders/" + fileName;
	const std::string c_src = readTextFile(c_shaderPath);
	if (c_src.empty()) {
		showError("[ERROR] missing shader file " + c_shaderPath);
		return nullptr;
	}

	std::cout << "[INFO] SPIR-V shader compilation: " << c_shaderPath << std::endl;
	shaderc::Compiler m_compiler;
	shaderc::CompileOptions m_opts;
	m_opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
	m_opts.SetTargetSpirv(shaderc_spirv_version_1_6);
	m_opts.SetOptimizationLevel(shaderc_optimization_level_performance);
	shaderc::CompilationResult m_result = m_compiler.CompileGlslToSpv(c_src, kind, fileName.c_str(), m_opts);

	if (m_result.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cerr << "[ERROR] shader compilation: " << m_result.GetErrorMessage() << std::endl;
		return nullptr;
	}
	std::vector<uint32_t> m_spv = { m_result.cbegin(), m_result.cend() };

	VkShaderModuleCreateInfo m_moduleCreateInfo {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = m_spv.size() * sizeof(uint32_t),
		.pCode = m_spv.data()
	};
	VkShaderModule m_shaderModule = nullptr;
	if (vkCreateShaderModule(m_device, &m_moduleCreateInfo, nullptr, &m_shaderModule) != VK_SUCCESS) {
		showError("[ERROR] shader module cannot be created");
		return nullptr;
	}
	return m_shaderModule;
}

bool SdlRenderer::createShaders() {
	if (m_vertShader = createShaderModule("shader.vert", shaderc_vertex_shader); !m_vertShader) {
		return false;
	}
	if (m_fragShader = createShaderModule("shader.frag", shaderc_fragment_shader); !m_fragShader) {
		return false;
	}
	return true;
}

VkPipeline SdlRenderer::createGraphicsPipeline() {
	VkPipelineLayoutCreateInfo m_pipelineLayoutInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0
	};

	if (vkCreatePipelineLayout(m_device, &m_pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		showError("[FAILED] pipeline layout creation failure");
		return nullptr;
	}

	const char *c_entryPoint = "main";
	std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = m_vertShader,
			.pName = c_entryPoint
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = m_fragShader,
			.pName = c_entryPoint
		}
	};

	VkPipelineVertexInputStateCreateInfo m_vertInputInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};

	VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	};

	VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.stencilTestEnable = VK_FALSE
	};

	VkPipelineViewportStateCreateInfo m_viewportInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = nullptr,
		.scissorCount = 1,
		.pScissors = nullptr
	};

	VkPipelineRasterizationStateCreateInfo m_rasterInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo m_multiSampleInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
	};

	VkPipelineColorBlendAttachmentState m_attachState {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo m_blendInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &m_attachState
	};

	std::vector<VkDynamicState> m_dynamicState {
		VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo m_dynamicStateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(m_dynamicState.size()),
		.pDynamicStates = m_dynamicState.data()
	};

	VkPipelineRenderingCreateInfo m_renderInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &c_swapchainFormat,
		.depthAttachmentFormat = c_depthFormat
	};

	VkGraphicsPipelineCreateInfo m_pipelineInfo {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &m_renderInfo,
		.stageCount = static_cast<uint32_t>(m_shaderStages.size()),
		.pStages = m_shaderStages.data(),
		.pVertexInputState = &m_vertInputInfo,
		.pInputAssemblyState = &m_inputAssemblyInfo,
		.pViewportState = &m_viewportInfo,
		.pRasterizationState = &m_rasterInfo,
		.pMultisampleState = &m_multiSampleInfo,
		.pDepthStencilState = &m_depthStencilInfo,
		.pColorBlendState = &m_blendInfo,
		.pDynamicState = &m_dynamicStateInfo,
		.layout = m_pipelineLayout,
		.renderPass = VK_NULL_HANDLE,
	};

	if (vkCreateGraphicsPipelines(m_device, nullptr, 1, &m_pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
		showError("[FAILED] pipeline creation failure");
		return nullptr;
	}
	return m_pipeline;
}

bool SdlRenderer::createSyncResources() {
	VkSemaphoreTypeCreateInfo m_semaphoreTypeInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = c_maxFramesInFlight
	};

	VkSemaphoreCreateInfo m_semaphoreInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &m_semaphoreTypeInfo
	};

	if (vkCreateSemaphore(m_device, &m_semaphoreInfo, nullptr, &m_timelineSemaphore) != VK_SUCCESS) {
		showError("[FAILED] timeline semaphore creation failure");
		return false;
	}

	for (FrameResources &res : m_frameResources) {
		VkSemaphoreCreateInfo m_semaphoreInfo {
		    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		if (vkCreateSemaphore(m_device, &m_semaphoreInfo, nullptr, &res.m_imageAcquiredSemaphore) != VK_SUCCESS) {
			showError("[ERROR] per-frame image-acquire semaphore cannot be created");
			return false;
		}
	}

	return true;
}

bool SdlRenderer::createCommandBuffers() {
	for (FrameResources &res : m_frameResources) {
		VkCommandPoolCreateInfo m_poolInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = m_gfxQueueFamIdx
		};

		if (vkCreateCommandPool(m_device, &m_poolInfo, nullptr, &res.m_commandPool) != VK_SUCCESS) {
			showError("[FAILED] command buffer pool failure");
			return false;
		}

		VkCommandBufferAllocateInfo m_cmdAllocInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = res.m_commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		if (vkAllocateCommandBuffers(m_device, &m_cmdAllocInfo, &res.m_commandBuffer) != VK_SUCCESS) {
			showError("[FAILED] command buffer allocation failure");
			return false;
		}
	}
	return true;
}

void SdlRenderer::Run() {
    m_running = true;

	while (m_running) {
		SDL_Event event { 0 };
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				m_running = false;
				break;
			}
			else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				m_width = event.window.data1;
				m_height = event.window.data2;
				break;
			}
		}

		Render();
	}
}

void SdlRenderer::Render() {
	if (m_requireSwapchainRecreate) {
		vkDeviceWaitIdle(m_device);
		destroySwapchain();
		createSwapchain(m_width, m_height);
		m_requireSwapchainRecreate = false;
	}

	const uint32_t c_frameResIndex = m_frameIndex++ % c_maxFramesInFlight;
	const uint64_t c_signalValue = m_nextSignalValue++;
	const uint64_t c_waitValue = c_signalValue - c_maxFramesInFlight;

	VkSemaphoreWaitInfo m_waitInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = &m_timelineSemaphore,
		.pValues = &c_waitValue
	};

	vkWaitSemaphores(m_device, &m_waitInfo, UINT64_MAX);

	FrameResources &m_res = m_frameResources[c_frameResIndex];
	vkResetCommandPool(m_device, m_res.m_commandPool, 0);

	VkSemaphore m_imageAcquireSemaphore = m_frameResources[c_frameResIndex].m_imageAcquiredSemaphore;

	uint32_t m_imageIndex = 0;
	VkResult m_acquireResult = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAcquireSemaphore, VK_NULL_HANDLE, &m_imageIndex);

	if (m_acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
		m_requireSwapchainRecreate = true;
		return;
	} else if (m_acquireResult == VK_SUBOPTIMAL_KHR) {
		m_requireSwapchainRecreate = true;
	}

	VkCommandBufferBeginInfo m_cmdBeginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(m_res.m_commandBuffer, &m_cmdBeginInfo);

	std::vector<VkImageMemoryBarrier2> m_layoutBarriers {
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.image = m_swapchainImages[m_imageIndex],
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		},
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, // both specified to control memory access at both stages (write)
			.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.image = m_depthImage,
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		}
	};

	VkDependencyInfo m_depInfo {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = static_cast<uint32_t>(m_layoutBarriers.size()),
		.pImageMemoryBarriers = m_layoutBarriers.data()
	};

	vkCmdPipelineBarrier2(m_res.m_commandBuffer, &m_depInfo);

	VkRenderingAttachmentInfo m_colorAttachInfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = m_swapchainImageViews[m_imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue {
		    .color{ {0.01f, 0.01f, 0.01f, 1} }
	    }
	};

	VkRenderingAttachmentInfo m_depthAttachInfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = m_depthImageView,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.clearValue { .depthStencil {1.0f, 0} }
	};

	VkRenderingInfo m_renderingInfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea {
			.offset{.x = 0, .y = 0},
			.extent{.width = m_swapchainWidth, .height = m_swapchainHeight}
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &m_colorAttachInfo,
		.pDepthAttachment = &m_depthAttachInfo
	};

	vkCmdBeginRendering(m_res.m_commandBuffer, &m_renderingInfo);
	{
		VkViewport viewport {
			.x = 0, .y = 0,
			.width = static_cast<float>(m_swapchainWidth),
			.height = static_cast<float>(m_swapchainHeight)
		};

		vkCmdSetViewport(m_res.m_commandBuffer, 0, 1, &viewport);

		VkRect2D m_scissor {
			.offset{.x = 0, .y = 0 },
			.extent{.width = m_swapchainWidth, .height = m_swapchainHeight}
		};

		vkCmdSetScissor(m_res.m_commandBuffer, 0, 1, &m_scissor);
		vkCmdBindPipeline(m_res.m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		vkCmdDraw(m_res.m_commandBuffer, 3, 1, 0, 0);
	}

	vkCmdEndRendering(m_res.m_commandBuffer);

	VkImageMemoryBarrier2 m_presentLayoutBarrier {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.image = m_swapchainImages[m_imageIndex],
		.subresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	VkDependencyInfo m_presentDepInfo {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &m_presentLayoutBarrier
	};

	vkCmdPipelineBarrier2(m_res.m_commandBuffer, &m_presentDepInfo);
	vkEndCommandBuffer(m_res.m_commandBuffer);

	VkSemaphoreSubmitInfo m_imageAcquireWaitInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = m_imageAcquireSemaphore,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	std::vector<VkSemaphoreSubmitInfo> m_semaphoreSignals {
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = m_renderCompleteSemaphores[m_imageIndex],
			.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
		},
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = m_timelineSemaphore,
			.value = c_signalValue,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
		}
	};

	VkCommandBufferSubmitInfo m_cmdSubmitInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = m_res.m_commandBuffer,
	};

	VkSubmitInfo2 m_submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &m_imageAcquireWaitInfo,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &m_cmdSubmitInfo,
		.signalSemaphoreInfoCount = static_cast<uint32_t>(m_semaphoreSignals.size()),
		.pSignalSemaphoreInfos = m_semaphoreSignals.data()
	};

	vkQueueSubmit2(m_gfxQueue, 1, &m_submitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR m_presentInfo {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderCompleteSemaphores[m_imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &m_imageIndex,
		.pResults = nullptr
	};

	vkQueuePresentKHR(m_gfxQueue, &m_presentInfo);
}

void SdlRenderer::destroySwapchain()
{
	for (VkImageView swapchainImgView : m_swapchainImageViews)
	{
		vkDestroyImageView(m_device, swapchainImgView, nullptr);
	}
	m_swapchainImageViews.clear();

	for (VkSemaphore &semaphore : m_renderCompleteSemaphores)
	{
		vkDestroySemaphore(m_device, semaphore, nullptr);
	}
	m_renderCompleteSemaphores.clear();

	if (m_swapchain)
	{
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
		m_swapchain = nullptr;
	}

	if (m_depthImageView)
	{
		vkDestroyImageView(m_device, m_depthImageView, nullptr);
		vmaDestroyImage(m_vmaAllocator, m_depthImage, m_depthImageAllocation);
		m_depthImageView = nullptr;
	}
}

void SdlRenderer::Shutdown() {
	vkDeviceWaitIdle(m_device);

	if (m_timelineSemaphore) {
		vkDestroySemaphore(m_device, m_timelineSemaphore, nullptr);
	}

	for (auto &res : m_frameResources) {
		vkDestroySemaphore(m_device, res.m_imageAcquiredSemaphore, nullptr);
		vkDestroyCommandPool(m_device, res.m_commandPool, nullptr);
	}

	if (m_pipelineLayout) {
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	}

	if (m_pipeline) {
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
	}

	if (m_vertShader) {
		vkDestroyShaderModule(m_device, m_vertShader, nullptr);
	}

	if (m_fragShader) {
		vkDestroyShaderModule(m_device, m_fragShader, nullptr);
	}

	destroySwapchain();

	if (m_vmaAllocator) {
		vmaDestroyAllocator(m_vmaAllocator);
	}

	if (m_surface) {
		vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
	}

	if (m_device) {
		vkDestroyDevice(m_device, nullptr);
	}

	if (m_vulkanInstance) {
		vkDestroyInstance(m_vulkanInstance, nullptr);
	}

	volkFinalize();

	if (m_window_ptr) {
		SDL_DestroyWindow(m_window_ptr);
	}

	SDL_Quit();
}

void SdlRenderer::showError(const std::string &errorMessasge) const {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "[ERROR]", errorMessasge.c_str(), m_window_ptr);
}
