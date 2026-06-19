void showError(SDL_Window *p_window, const std::string &p_errorMessasge) {}

std::string readTextFile(const std::string &p_filePath)
{
	std::ifstream m_infile(p_filePath);
	if (m_infile.is_open())
	{
		std::stringstream m_buffer;
		m_buffer << m_infile.rdbuf();
		const std::string output = m_buffer.str();
		m_infile.close();
		return output;
	}
	return std::string();
}
