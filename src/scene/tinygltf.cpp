// Conan's tinygltf package is header-only. Compile its implementation exactly once.
// Keep this STB instance separate from FileManager's static, vertically-flipped loader.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
