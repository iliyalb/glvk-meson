# glvk

## About

OpenGL + Vulkan multi-API project

## Setup

### Linux

Use linux package manager to install `g++`, `glfw`, and `glad`.

> [!OPTIONAL]
> copy [glad](https://glad.dav1d.de/) web service include directories `glad` and `KHR` to `/usr/include/`

### Windows

Install latest GPU driver, [cmake](https://cmake.org/download/), [glfw](https://www.glfw.org/download.html), [glad](https://glad.dav1d.de/) and Visual Studio.

Make sure `x64` build selected, from project -> properties set platform to all platforms then under configuration properties -> `VC++` directories open include directories point to the include directory that has `GLFW`, `glad`, and `KHR` inside. Do the same for library directories pointing to the `lib` directory that has `glfw3.lib`. Next up go to the linker -> input and write `glfw3.lib` and `opengl32.lib` in additional dependencies. Finally drag and drop `glad.c` to the source files of solution explorer.

## Build

First time setup:

```sh
mkdir -p build && meson setup build
```

Changes to `meson.build`:

```sh
meson setup --reconfigure build && ninja install -C build
```

Build and run:

```sh
ninja -C build && (cd ./build && ./glvk.out)
```

## License

This is free and unencumbered software released into the public domain under The Unlicense.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.

See [UNLICENSE](LICENSE) for full details.
