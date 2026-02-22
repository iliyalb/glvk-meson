#pragma once

class GlRenderer {
public:
    GlRenderer();
    void createWindow(const char* p_name = "GL\0", const int p_width = 1280, const int p_height = 720);
    GLFWwindow* getWindow() const { return m_window_ptr; }
    ~GlRenderer();

private:
    GLFWwindow *m_window_ptr;
};
