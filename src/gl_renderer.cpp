GlRenderer::GlRenderer() {}

void GlRenderer::createWindow(const char* p_name, const int p_width, const int p_height) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window_ptr = glfwCreateWindow(p_width, p_height, p_name, nullptr, nullptr);
	glfwMakeContextCurrent(m_window_ptr);
	gladLoadGL();
	glViewport(0, 0, p_width, p_height);
	glClearColor(0.05f, 0.15f, 0.20f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(m_window_ptr);
}

GlRenderer::~GlRenderer() {}
