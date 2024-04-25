# Shady
[![Windows](https://github.com/JacobDomagala/Shady/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/JacobDomagala/Shady/actions/workflows/windows.yml?query=branch%3Amaster)
[![Ubuntu](https://github.com/JacobDomagala/Shady/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/JacobDomagala/Shady/actions/workflows/ubuntu.yml?query=branch%3Amaster)

------------------------------------------------------------

Shady is a 3D rendering engine written in modern C++ and Vulkan. It's not meant to be a fully fledged rendering/game engine, it's just a fun project I'm doing in my free time. </br>

This project is in an early stage so it's still missing many basic features. For current roadmap/planned features see [Github Project](https://github.com/users/JacobDomagala/projects/3/views/1) </br>

Below you can see famous [Sponza](https://en.wikipedia.org/wiki/Sponza_Palace) scene rendered using Shady (click to see YouTube video): <br>
[![Sponza](https://github.com/JacobDomagala/Shady/wiki/Youtube.png)](https://www.youtube.com/watch?v=LZlHqkR0CQ0)

------------------------------------------------------------
## Building

Shady is CMake/Conan based project working both on Linux (Ubuntu) and Windows. To build it, you will need at least C++20 compiler and CMake version 3.22. </br>
While most of the dependencies will be handled by Conan, it's required that you have Vulkan installed on your machine.

Typical build process would look like this:
```bash
# Create build directory
mkdir build && cd build

conan profile detect
conan install .. -of=build --build=missing -s compiler.cppstd=20

# Generate build system for Windows/Linux
cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake ..

# Build
cmake --build .
```

## Youtube
For past and future video logs, please visit my [Youtube](https://www.youtube.com/@Jacob.Domagala) channel. <br>
[![Playlist](https://img.youtube.com/vi/LZlHqkR0CQ0/0.jpg)](https://www.youtube.com/watch?v=LZlHqkR0CQ0&list=PLRLVUsGGaSH8GcSjxOiAQBRWuFpVtWVOp "YouTube Playlist")
