_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")
#ifndef PCH_H
#define PCH_H

// standard
#include <algorithm>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// vendor
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <shaderc/shaderc.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// project
#include <vk_utility.hpp>
#include <vk_renderer.hpp>
#include <gl_renderer.hpp>

#endif
_Pragma("GCC diagnostic pop")
