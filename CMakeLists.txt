cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME Shady)
project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX)

set(SOURCES src/Main.cpp src/SkyBox.h src/SkyBox.cpp src/Light.h src/Light.cpp src/EventListener.h src/EventListener.cpp
            src/SHADERS/Shader.h src/SHADERS/Shader.cpp
            src/MODEL/Mesh.h src/MODEL/Mesh.cpp src/MODEL/Model.h src/MODEL/Model.cpp src/MODEL/Texture.h src/MODEL/Texture.cpp
            src/DISPLAY/Camera.h src/DISPLAY/Camera.cpp src/DISPLAY/Display.h src/DISPLAY/Display.cpp src/DISPLAY/Movement.h src/DISPLAY/Movement.cpp
            src/CLOCK/Clock.h src/CLOCK/Clock.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(lib)

target_link_libraries (${PROJECT_NAME} glm glfw GLAD STB_IMAGE)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)