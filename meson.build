# Project setup (C for glad.c, C++ for app)
project('glvk', ['c', 'cpp'],
  default_options : ['cpp_std=c++20'])

inc = include_directories('include')

# vendor headers linking against system glfw
glfw_dep = dependency('glfw3', required : true)
dl_dep = meson.get_compiler('cpp').find_library('dl', required : true)

sources = [
  'src/main.cpp',
  'src/glad.c',
]

executable('glvk.out',
  sources : sources,
  include_directories : inc,
  dependencies : [glfw_dep, dl_dep])
