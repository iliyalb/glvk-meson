_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")
#ifdef __clang__
_Pragma("clang diagnostic ignored \"-Wpadded\"")
#endif
#ifndef PCH_H
#define PCH_H
#define VK_NO_PROTOTYPES

// standard
#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// vendor
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <shaderc/shaderc.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// project
#include <sdl_utility.hpp>
#include <sdl_renderer.hpp>
#include <vk_utility.hpp>
#include <vk_renderer.hpp>
#include <gl_renderer.hpp>

#endif
#ifdef __clang__
_Pragma("clang diagnostic pop")
#else
_Pragma("GCC diagnostic pop")
#endif
