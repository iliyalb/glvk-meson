VkRenderer::VkRenderer() {}

int VkRenderer::initialize(GLFWwindow *p_newWindow) {
    m_window_ptr = p_newWindow;
    try {
        createInstance();
        getPhysicalDevice();
        createLogicalDevice();
    }
    catch (const std::runtime_error &p_error) {
        throw &p_error;
        return EXIT_FAILURE;
    }
    return 0;
}

void VkRenderer::createInstance() {
    VkApplicationInfo c_appInfo = {};
    c_appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    c_appInfo.pApplicationName = "vulkan application";
    c_appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    c_appInfo.pEngineName = "custom";
    c_appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    c_appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo c_createInfo = {};
    c_createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    c_createInfo.pApplicationInfo = &c_appInfo;

    std::vector<const char*> m_instanceExtensions = std::vector<const char*>();
    uint32_t c_glfwExtensionCount = 0;
    const char** c_glfwExtensions_ptr = glfwGetRequiredInstanceExtensions(&c_glfwExtensionCount);

    for(size_t i =0; i < c_glfwExtensionCount; i++) {
        m_instanceExtensions.push_back(c_glfwExtensions_ptr[i]);
    }

    if(!checkInstanceExtensionSupport(&m_instanceExtensions)) {
        throw std::runtime_error("[ERROR] instance lacks extension support");
    }

    c_createInfo.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensions.size());
    c_createInfo.ppEnabledExtensionNames = m_instanceExtensions.data();
    c_createInfo.enabledLayerCount = 0;
    c_createInfo.ppEnabledLayerNames = nullptr;

    VkResult c_result = vkCreateInstance(&c_createInfo, nullptr, &m_instance);
    if(c_result != VK_SUCCESS) {
        throw std::runtime_error("[FAILED] instance creation unsuccessful");
    }
}

void VkRenderer::getPhysicalDevice() {
    uint32_t c_deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &c_deviceCount, nullptr);
    std::vector<VkPhysicalDevice> m_deviceList(c_deviceCount);
    if(c_deviceCount == 0) {
        throw std::runtime_error("[ERROR] instance does not have graphic accelerator");
    }
    vkEnumeratePhysicalDevices(m_instance, &c_deviceCount, m_deviceList.data());

    auto c_deviceIterator = std::ranges::find_if(m_deviceList, [this](const VkPhysicalDevice& p_thisDevice) {
            return checkCompatibleDevice(p_thisDevice);
    });

    if (c_deviceIterator != m_deviceList.end()) {
        m_device.m_physical = *c_deviceIterator;
    } else {
        throw std::runtime_error("[ERROR] no compatible vulkan capable device found");
    }
}

void VkRenderer::createLogicalDevice() {
    const float c_priority = 1.0f;
    QueueFamily c_indices = getQueueFamilies(m_device.m_physical);

    VkDeviceQueueCreateInfo c_queueCreateInfo = {};
    c_queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    c_queueCreateInfo.queueFamilyIndex = c_indices.m_graphicsFamily;
    c_queueCreateInfo.queueCount = 1;
    c_queueCreateInfo.pQueuePriorities = &c_priority;

    VkDeviceCreateInfo c_deviceCreateInfo = {};
    c_deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    c_deviceCreateInfo.queueCreateInfoCount = 1;
    c_deviceCreateInfo.pQueueCreateInfos = &c_queueCreateInfo;
    c_deviceCreateInfo.enabledExtensionCount = 0;
    c_deviceCreateInfo.ppEnabledExtensionNames = nullptr;
    //c_deviceCreateInfo.enabledLayerCount = 0; -- deprecated

    VkPhysicalDeviceFeatures c_deviceFeatures = {};
    c_deviceCreateInfo.pEnabledFeatures = &c_deviceFeatures;
    VkResult c_result = vkCreateDevice(m_device.m_physical, &c_deviceCreateInfo, nullptr, &m_device.m_logical);
    if(c_result != VK_SUCCESS) {
        throw std::runtime_error("[FAILED] logical device creation unsuccessful");
    }
    vkGetDeviceQueue(m_device.m_logical, c_indices.m_graphicsFamily, 0, &m_graphicsQueue);
}

void VkRenderer::createWindow(const char* p_name, const int p_width, const int p_height) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window_ptr = glfwCreateWindow(p_width, p_height, p_name, nullptr, nullptr);
}

bool VkRenderer::checkInstanceExtensionSupport(std::vector<const char*> *extensions) {
    uint32_t c_extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &c_extensionCount, nullptr);
    std::vector<VkExtensionProperties> m_extensionProperties(c_extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &c_extensionCount, m_extensionProperties.data());

    return std::ranges::all_of(*extensions, [&m_extensionProperties](std::string_view p_extension) {
        return std::ranges::any_of(m_extensionProperties, [p_extension](const VkExtensionProperties &p_prop) {
            return p_extension == p_prop.extensionName;
        });
    });
    return true;
}

bool VkRenderer::checkCompatibleDevice(VkPhysicalDevice p_device) {
    QueueFamily m_indices = getQueueFamilies(p_device);
    return m_indices.isValid();
}

QueueFamily VkRenderer::getQueueFamilies(VkPhysicalDevice p_physicalDevice) {
    QueueFamily m_indices;
    uint32_t c_queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(p_physicalDevice, &c_queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> m_queueFamilies(c_queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(p_physicalDevice, &c_queueFamilyCount, m_queueFamilies.data());

    auto c_index = std::ranges::find_if(m_queueFamilies, [](const VkQueueFamilyProperties &p_qf) {
            return p_qf.queueCount > 0 && (p_qf.queueFlags & VK_QUEUE_GRAPHICS_BIT);
    });

    if (c_index != m_queueFamilies.end()) {
        m_indices.m_graphicsFamily = std::distance(m_queueFamilies.begin(), c_index);
    }
    return m_indices;
}

void VkRenderer::destroy() {
    vkDestroyDevice(m_device.m_logical, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

VkRenderer::~VkRenderer() {}
