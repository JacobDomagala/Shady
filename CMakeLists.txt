cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME Shady)
project(${PROJECT_NAME} VERSION 0.0.1)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

add_executable(${PROJECT_NAME}
    # global namespace (legacy files)
    src/Main.cpp

    # app
    src/app/window.hpp src/app/window.cpp
    # app/input
    src/app/input/event.hpp src/app/input/input_manager.hpp src/app/input/input_manager.cpp src/app/input/input_listener.hpp

    # trace
    src/trace/logger.hpp src/trace/logger.impl.hpp src/trace/logger.cpp

    # time
    src/time/timer.hpp src/time/timer.cpp src/time/scoped_timer.hpp src/time/scoped_timer.cpp src/time/utils.hpp src/time/utils.cpp

    # render
    src/render/render_command.hpp src/render/render_command.cpp src/render/renderer_api.hpp src/render/renderer_api.cpp
    src/render/vertex_array.hpp src/render/vertex_array.cpp src/render/buffer.hpp src/render/buffer.cpp
    src/render/renderer.hpp src/render/renderer.cpp src/render/context.hpp src/render/context.cpp
    src/render/texture.hpp src/render/texture.cpp src/render/shader.hpp src/render/shader.cpp
    src/render/camera.hpp src/render/camera.cpp src/render/orthographic_camera.hpp src/render/orthographic_camera.cpp
    src/render/renderer_2D.hpp src/render/renderer_2D.cpp src/render/renderer_3D.hpp src/render/renderer_3D.cpp
    src/render/vertex.hpp src/render/framebuffer.hpp src/render/framebuffer.cpp

    # render/opengl
    src/render/opengl/opengl_renderer_api.hpp src/render/opengl/opengl_renderer_api.cpp src/render/opengl/opengl_vertex_array.hpp
    src/render/opengl/opengl_vertex_array.cpp src/render/opengl/opengl_context.hpp src/render/opengl/opengl_context.cpp
    src/render/opengl/opengl_buffer.hpp src/render/opengl/opengl_buffer.cpp src/render/opengl/opengl_texture.hpp src/render/opengl/opengl_texture.cpp
    src/render/opengl/opengl_shader.hpp src/render/opengl/opengl_shader.cpp src/render/opengl/opengl_framebuffer.hpp src/render/opengl/opengl_framebuffer.cpp

    # scene
    src/scene/mesh.hpp src/scene/mesh.cpp src/scene/model.hpp src/scene/model.cpp src/scene/light.hpp src/scene/light.cpp
    src/scene/skybox.hpp src/scene/skybox.cpp

    # utils
    src/utils/file_manager.hpp src/utils/file_manager.cpp src/utils/assert.hpp src/utils/assert.cpp
)

add_subdirectory(lib)

target_include_directories(${PROJECT_NAME}
                    PRIVATE src src/app src/app/input src/trace src/render src/utils src/scene src/time)
target_link_libraries (${PROJECT_NAME} PRIVATE glm glfw glad stb_image assimp fmt)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
