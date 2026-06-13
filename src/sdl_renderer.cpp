#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION


bool SdlRenderer::Initialize() {
   	SDL_InitSubSystem(SDL_INIT_VIDEO);
	m_window_ptr = SDL_CreateWindow("Vulkan", m_width, m_height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (!m_window_ptr) {
		showError("[FAILED] window creation unsuccessful");
		return false;
	}

	/* if (!initializeVulkan()) {
		return false;
		} */

	return true;
}

void SdlRenderer::Run() {
    m_running = true;

	while (m_running) {
		SDL_Event event{ 0 };
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

		// render();
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

	// destroySwapchain();

	// VMA
	if (m_vmaAllocator) {
		// vmaDestroyAllocator(m_vmaAllocator);
	}

	// cleanup Vulkan
	if (m_surface) {
		vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
	}
	if (m_device) {
		vkDestroyDevice(m_device, nullptr);
	}
	if (m_vulkanInstance) {
		vkDestroyInstance(m_vulkanInstance, nullptr);
	}

	// volkFinalize();

	if (m_window_ptr) {
		SDL_DestroyWindow(m_window_ptr);
	}
	SDL_Quit();
}

void SdlRenderer::showError(const std::string &errorMessasge) const {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "[ERROR]", errorMessasge.c_str(), m_window_ptr);
}
