cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME Shady)
project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

set(SOURCES src/Main.cpp src/SkyBox.h src/SkyBox.cpp src/Light.h src/Light.cpp src/EventListener.h src/EventListener.cpp
            src/SHADERS/Shader.h src/SHADERS/Shader.cpp
            src/MODEL/Mesh.h src/MODEL/Mesh.cpp src/MODEL/Model.h src/MODEL/Model.cpp src/MODEL/Texture.h src/MODEL/Texture.cpp
            src/DISPLAY/Camera.h src/DISPLAY/Camera.cpp src/DISPLAY/Movement.h src/DISPLAY/Movement.cpp
            src/CLOCK/Clock.h src/CLOCK/Clock.cpp
            src/app/window.hpp src/app/window.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(lib)

target_include_directories(${PROJECT_NAME} PRIVATE src src/app src/MODEL src/CLOCK src/DISPLAY src/SHADERS)
target_link_libraries (${PROJECT_NAME} PRIVATE glm glfw GLAD STB_IMAGE assimp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# TODO: Is there a way to do it other way?
# Copy assimp .dll file to build dir
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${OUTPUT_PATH}/lib/Assimp/Assimp/bin ${OUTPUT_PATH})