cmake_minimum_required(VERSION 3.27)

set(PROJECT_NAME Shady)
project(${PROJECT_NAME} VERSION 0.0.2)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

add_definitions( -DCMAKE_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

include(cmake/compile_warnings.cmake)
include(cmake/clang_format.cmake)
include(cmake/util_functions.cmake)
set_target_warnings(project_warnings)

add_executable(${PROJECT_NAME}
    # app
    src/app/window.hpp src/app/window.cpp src/app/main.cpp src/app/shady.hpp src/app/shady.cpp

    # app/input
    src/app/input/event.hpp src/app/input/input_manager.hpp src/app/input/input_manager.cpp src/app/input/input_listener.hpp

    # app/gui
    src/app/gui/gui.hpp src/app/gui/gui.cpp

    # trace
    src/trace/logger.hpp src/trace/logger.impl.hpp src/trace/logger.cpp src/trace/formatter_types.hpp

    # time
    src/time/timer.hpp src/time/timer.cpp src/time/scoped_timer.hpp src/time/scoped_timer.cpp src/time/utils.hpp src/time/utils.cpp

    # render/vulkan
    "src/render/renderer.hpp" "src/render/renderer.cpp" "src/render/shader.hpp" "src/render/shader.cpp"
    "src/render/buffer.hpp" "src/render/buffer.cpp" "src/render/texture.hpp" "src/render/texture.cpp"
    "src/render/command.hpp" "src/render/command.cpp" "src/render/common.hpp" "src/render/common.cpp"
    "src/render/vertex.hpp" "src/render/types.hpp" "src/render/framebuffer.hpp" "src/render/framebuffer.cpp"
    "src/render/deferred_pipeline.hpp" "src/render/deferred_pipeline.cpp"
    "src/render/profiler.hpp" "src/render/profiler.cpp"

    # scene
    src/scene/mesh.hpp src/scene/mesh.cpp src/scene/model.hpp src/scene/model.cpp src/scene/light.hpp src/scene/light.cpp
    src/scene/skybox.hpp src/scene/skybox.cpp src/scene/scene.hpp src/scene/scene.cpp
    src/scene/camera.hpp src/scene/camera.cpp src/scene/orthographic_camera.hpp src/scene/orthographic_camera.cpp
    src/scene/perspective_camera.hpp src/scene/perspective_camera.cpp

    # utils
    src/utils/file_manager.hpp src/utils/file_manager.cpp src/utils/assert.hpp src/utils/assert.cpp
)

add_clang_format_target(${PROJECT_NAME})

find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)
find_package(stb REQUIRED)
find_package(TinyGLTF 2.9 CONFIG REQUIRED)

find_package(Vulkan REQUIRED)

# Conan supplies header-only TinyGLTF; isolate its implementation and STB from engine warnings.
add_library(shady_tinygltf STATIC src/scene/tinygltf.cpp)
target_link_libraries(shady_tinygltf PUBLIC TinyGLTF::TinyGLTF)
target_compile_features(shady_tinygltf PUBLIC cxx_std_20)
target_compile_definitions(shady_tinygltf PUBLIC TINYGLTF_NOEXCEPTION JSON_NOEXCEPTION)

add_library(shady_gltf STATIC src/scene/gltf_loader.cpp src/scene/gltf_loader.hpp)
target_include_directories(shady_gltf PUBLIC src)
target_link_libraries(shady_gltf PUBLIC shady_tinygltf glm::glm Vulkan::Headers)
target_link_libraries(shady_gltf PRIVATE project_warnings)
target_compile_definitions(shady_gltf PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_include_directories(${PROJECT_NAME}
                    PRIVATE src src/app src/app/input src/trace src/render src/utils src/scene src/time)
target_link_libraries_system (${PROJECT_NAME} PRIVATE fmt::fmt glfw imgui::imgui glm::glm Vulkan::Vulkan stb::stb)
target_link_libraries(${PROJECT_NAME} PRIVATE project_warnings shady_gltf)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FMT_USE_CONSTEXPR_CONSTRUCTION=0
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)
if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-O0;-g3;-fno-omit-frame-pointer>)
endif()

if(MSVC)
    # Remove CMake's default /EHsc before applying the engine's no-exception policy.
    string(REGEX REPLACE "(^|[ \t])[-/]EH[a-zA-Z-]+" "\\1" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

foreach(target ${PROJECT_NAME} shady_gltf shady_tinygltf)
    if(MSVC)
        target_compile_options(${target} PRIVATE /EHs-c-)
        target_compile_definitions(${target} PRIVATE _HAS_EXCEPTIONS=0)
    else()
        target_compile_options(${target} PRIVATE -fno-exceptions)
    endif()
endforeach()

include(cmake/compile_shaders.cmake)
