# Shady
[![Windows](https://github.com/JacobDomagala/Shady/actions/workflows/windows.yml/badge.svg)](https://github.com/JacobDomagala/Shady/actions/workflows/windows.yml)
[![Ubuntu](https://github.com/JacobDomagala/Shady/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/JacobDomagala/Shady/actions/workflows/ubuntu.yml)

------------------------------------------------------------

Shady is a 3D rendering engine written in modern C++ and OpenGL. It's not meant to be a fully fledged rendering/game engine, it's just a fun project I'm doing in my free time. </br>

This project is in an early stage so it's still missing many basic features. For current roadmap/planned features see [Github Project](https://github.com/JacobDomagala/Shady/projects/2) </br>

Below you can see famous [Sponza](https://en.wikipedia.org/wiki/Sponza_Palace) scene rendered using Shady.
![Sponza](https://github.com/JacobDomagala/Shady/wiki/screenshot_1.PNG)

------------------------------------------------------------
## Building

Shady is CMake based project working both on Linux (Ubuntu) and Windows. To build it, you will need at least C++17 compiler and CMake version 3.13.  </br>
Typical build process would look like this:
```bash
# Create build directory
mkdir build && cd build

# Generate build system for Windows/Linux
cmake -G "Visual Studio 16 2019" ..
cmake -G "Ninja" ..

# Build
cmake --build .
```
