VkRenderer g_vkRenderer;

int main() {
    glfwInit();
    g_vkRenderer.createWindow();
    GLFWwindow* window_ptr = g_vkRenderer.getWindow();
    g_vkRenderer.initialize(window_ptr);
    while(!glfwWindowShouldClose(window_ptr)) glfwPollEvents();
    g_vkRenderer.destroy();
   	glfwDestroyWindow(window_ptr);
	glfwTerminate();
    return 0;
}
