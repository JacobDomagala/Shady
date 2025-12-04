#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-Debug}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${SOURCE_DIR}/build"
GENERATOR="Visual Studio 18 2026"
GENERATOR_PLATFORM="x64"
VULKAN_SDK="C:/VulkanSDK/1.4.350.0"
VULKAN_INCLUDE_DIR="${VULKAN_SDK}/Include"
VULKAN_LIBRARY="${VULKAN_SDK}/Lib/vulkan-1.lib"

# Ensure Conan is in PATH for this session
export PATH="$PATH:/c/Users/Jakub/AppData/Roaming/Python/Python312/Scripts"
export VULKAN_SDK

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
    MSVC_RUNTIME_TYPE="Debug"
else
    MSVC_RUNTIME_TYPE="Release"
fi

mkdir -p "${BUILD_DIR}"

echo "== conan install =="
conan install "${SOURCE_DIR}" -of "${BUILD_DIR}" --build=missing \
    -s:h "build_type=${BUILD_TYPE}" \
    -s:h "compiler=msvc" \
    -s:h "compiler.version=194" \
    -s:h "compiler.cppstd=20" \
    -s:h "compiler.runtime=dynamic" \
    -s:h "compiler.runtime_type=${MSVC_RUNTIME_TYPE}" \
    -s:b "compiler=msvc" \
    -s:b "compiler.version=194" \
    -s:b "compiler.cppstd=20" \
    -s:b "compiler.runtime=dynamic" \
    -s:b "compiler.runtime_type=Release" \
    -c "tools.cmake.cmaketoolchain:generator=${GENERATOR}"

echo "== cmake configure =="
cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${BUILD_DIR}/conan_toolchain.cmake" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DVulkan_INCLUDE_DIR="${VULKAN_INCLUDE_DIR}" \
    -DVulkan_LIBRARY="${VULKAN_LIBRARY}" \
    -G "${GENERATOR}" \
    -A "${GENERATOR_PLATFORM}" \
    --fresh

echo "== build =="
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"
