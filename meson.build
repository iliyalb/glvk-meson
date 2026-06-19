# Project setup (C for glad.c, C++ for app)
project('glvk', ['c', 'cpp'],
  default_options : ['cpp_std=c++26'])

inc = include_directories('include')

# vendor headers linking system libraries
glfw_dep = dependency('glfw3', required : true)
sdl3_dep = dependency('sdl3', required : true)
shaderc_dep = dependency('shaderc', required : true)
dl_dep = meson.get_compiler('cpp').find_library('dl', required : true)
volk_dep = dependency('volk', required : true)
vulkan_dep = dependency('vulkan', required : true)

sources = [
  'src/main.cpp',
  'src/glad.c',
  'src/sdl_renderer.cpp',
  'src/sdl_utility.cpp',
  'src/gl_renderer.cpp'
]

executable('glvk.out',
  sources : sources,
  include_directories : inc,
  dependencies : [
    glfw_dep, sdl3_dep, shaderc_dep, dl_dep, volk_dep, vulkan_dep
  ],
  cpp_pch : 'include/pch.hpp')
