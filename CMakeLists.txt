cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME Shady)
project(${PROJECT_NAME} VERSION 0.0.2)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

set(ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(SHADERS_PATH ${ASSETS_PATH}/shaders)


add_definitions( -DCMAKE_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

include(cmake/compile_warnings.cmake)
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

    # scene
    src/scene/mesh.hpp src/scene/mesh.cpp src/scene/model.hpp src/scene/model.cpp src/scene/light.hpp src/scene/light.cpp
    src/scene/skybox.hpp src/scene/skybox.cpp src/scene/scene.hpp src/scene/scene.cpp
    src/scene/camera.hpp src/scene/camera.cpp src/scene/orthographic_camera.hpp src/scene/orthographic_camera.cpp
    src/scene/perspective_camera.hpp src/scene/perspective_camera.cpp

    # utils
    src/utils/file_manager.hpp src/utils/file_manager.cpp src/utils/assert.hpp src/utils/assert.cpp
)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_subdirectory(lib)
find_package(Vulkan REQUIRED)

target_include_directories(${PROJECT_NAME}
                    PRIVATE src src/app src/app/input src/trace src/render src/utils src/scene src/time)
target_link_libraries_system (${PROJECT_NAME} PRIVATE CONAN_PKG::fmt CONAN_PKG::glfw CONAN_PKG::assimp CONAN_PKG::imgui CONAN_PKG::glm Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE project_warnings)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# include(cmake/compile_shaders.cmake)
# compile_shader(SOURCE_FILE "${SHADERS_PATH}/default/default.vert"  OUTPUT_FILE_NAME "${SHADERS_PATH}/default/vert.spv")
# compile_shader(SOURCE_FILE "${SHADERS_PATH}/default/default.frag"  OUTPUT_FILE_NAME "${SHADERS_PATH}/default/frag.spv")

