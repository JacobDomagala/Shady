FIND_AND_DOWNLOAD_GIT_RESOURCE(imgui https://github.com/ocornut/imgui)

set(HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h")

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp")

add_library(imgui STATIC ${HEADERS} ${SOURCES})
target_include_directories(imgui SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/")
