#pragma once

class VkRenderer {
public:
    VkRenderer();
    int initialize(GLFWwindow *p_newWindow);
    void destroy();
    void createWindow(const char* p_name = "GLVK\0", const int p_width = 1280, const int p_height = 720);
    GLFWwindow* getWindow() const { return m_window_ptr; }
    ~VkRenderer();

private:
    GLFWwindow *m_window_ptr;
    VkInstance m_instance;
    class {
        public:
            VkPhysicalDevice m_physical;
            VkDevice m_logical;
    } m_device;
    VkQueue m_graphicsQueue;

    void createInstance();
    void getPhysicalDevice();
    void createLogicalDevice();

    bool checkInstanceExtensionSupport(std::vector<const char*> *p_extensions);
    bool checkCompatibleDevice(VkPhysicalDevice p_device);
    QueueFamily getQueueFamilies(VkPhysicalDevice p_physicalDevice);
};
