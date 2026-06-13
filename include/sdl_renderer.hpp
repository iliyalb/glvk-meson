#pragma once

class SDL_Window;
struct VmaAllocator_T;
struct VmaAllocation_T;
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

struct FrameResources
{
	VkCommandPool m_commandPool = nullptr;
	VkCommandBuffer m_commandBuffer = nullptr;
	VkSemaphore m_imageAcquiredSemaphore = nullptr;
};

class SdlRenderer
{
    // info
    constexpr static uint32_t c_vulkanVersion{ VK_API_VERSION_1_4 };
	constexpr static uint32_t c_maxFramesInFlight{ 2 };
	constexpr static VkFormat c_swapchainFormat{ VK_FORMAT_B8G8R8A8_SRGB };
	constexpr static VkFormat c_depthFormat{ VK_FORMAT_D32_SFLOAT };

	// window
	SDL_Window* m_window_ptr = nullptr;
	uint32_t m_width = 1280;
	uint32_t m_height = 720;
	bool m_running = false;
	uint64_t m_frameIndex = 0;
	uint64_t m_nextSignalValue = c_maxFramesInFlight + 1;

	// vulkan
	VkInstance m_vulkanInstance = nullptr;
	VkPhysicalDevice m_physicalDevice = nullptr;
	VkDevice m_device = nullptr;
	VkSurfaceKHR m_surface = nullptr;
	VmaAllocator m_vmaAllocator = nullptr;

	// queue
	uint32_t m_gfxQueueFamIdx = UINT32_MAX;
	VkQueue m_gfxQueue = nullptr;

	// swapchain
	VkSwapchainKHR m_swapchain = nullptr;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkSemaphore> m_renderCompleteSemaphores;
	bool m_requireSwapchainRecreate = false;
	uint32_t m_swapchainWidth = 0;
	uint32_t m_swapchainHeight = 0;

	// image
	VkImage m_depthImage = nullptr;
	VkImageView m_depthImageView = nullptr;
	VmaAllocation m_depthImageAllocation = nullptr;

	// pipeline
	VkPipelineLayout m_pipelineLayout = nullptr;
	VkPipeline m_pipeline = nullptr;

	// shader
	VkShaderModule m_vertShader = nullptr;
	VkShaderModule m_fragShader = nullptr;

	// frame
	VkSemaphore m_timelineSemaphore = nullptr;
	std::array<FrameResources, c_maxFramesInFlight> m_frameResources;

	void showError(const std::string& p_errorMessage) const;
	bool initializeVulkan();
	bool createVulkanInstance();
	bool createSurface();
	VkPhysicalDevice findPhysicalDevice();
	bool findGraphicsQueue();
	bool createDevice(VkPhysicalDevice p_physicalDevice);
	bool initializeVMA();
	bool createSwapchain(uint32_t p_width, uint32_t p_height);
	void destroySwapchain();
	VkShaderModule createShaderModule(const std::string& p_fileName, shaderc_shader_kind p_kind) const;
	bool createShaders();
	VkPipeline createGraphicsPipeline();
	bool createSyncResources();
	bool createCommandBuffers();
	void Render();

public:
	bool Initialize();
	void Shutdown();
	void Run();
};
