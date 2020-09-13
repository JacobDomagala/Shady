cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME Shady)
project(${PROJECT_NAME} VERSION 0.0.1)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

set(SOURCES src/Main.cpp src/SkyBox.h src/SkyBox.cpp src/Light.h src/Light.cpp src/EventListener.h src/EventListener.cpp
            src/SHADERS/Shader.h src/SHADERS/Shader.cpp
            src/MODEL/Mesh.h src/MODEL/Mesh.cpp src/MODEL/Model.h src/MODEL/Model.cpp src/MODEL/Texture.h src/MODEL/Texture.cpp
            src/DISPLAY/Camera.h src/DISPLAY/Camera.cpp src/DISPLAY/Movement.h src/DISPLAY/Movement.cpp
            src/app/window.hpp src/app/window.cpp
            src/trace/logger.hpp
            src/time/timer.hpp src/time/timer.cpp src/time/scoped_timer.hpp src/time/scoped_timer.cpp src/time/utils.hpp src/time/utils.cpp
            src/render/render_command.hpp src/render/render_command.cpp src/render/renderer_api.hpp src/render/renderer_api.cpp
            src/render/vertex_array.hpp src/render/vertex_array.cpp src/render/buffer.hpp src/render/buffer.cpp
            src/render/renderer.hpp src/render/renderer.cpp src/render/context.hpp src/render/context.cpp
            src/render/openGL/openGL_renderer_api.hpp src/render/openGL/openGL_renderer_api.cpp src/render/openGL/openGL_vertex_array.hpp
            src/render/openGL/openGL_vertex_array.cpp src/render/openGL/openGL_context.hpp src/render/openGL/openGL_context.cpp
            src/render/openGL/openGL_buffer.hpp src/render/openGL/openGL_buffer.cpp
            src/utils/file_manager.hpp src/utils/file_manager.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(lib)

target_include_directories(${PROJECT_NAME} PRIVATE src src/app src/trace src/render src/utils src/MODEL src/time src/DISPLAY src/SHADERS)
target_link_libraries (${PROJECT_NAME} PRIVATE glm glfw glad stb_image assimp fmt)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
