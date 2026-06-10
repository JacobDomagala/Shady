function(compile_shader)
    set(options "")
    set(oneValueArgs SOURCE_FILE OUTPUT_FILE_NAME)
    set(multiValueArgs DEFINES)
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_SOURCE_FILE)
        message(FATAL_ERROR "compile_shader: SOURCE_FILE argument missing")
    endif()

    if (NOT params_OUTPUT_FILE_NAME)
        message(FATAL_ERROR "compile_shader: OUTPUT_FILE_NAME argument missing")
    endif()

    add_custom_command(
        OUTPUT "${params_OUTPUT_FILE_NAME}"
        COMMAND ${Vulkan_GLSLC_EXECUTABLE}
                ${params_DEFINES}
                "${params_SOURCE_FILE}"
                -o "${params_OUTPUT_FILE_NAME}"
        DEPENDS "${params_SOURCE_FILE}"
        COMMENT "Compiling shader ${params_SOURCE_FILE}"
        VERBATIM
    )

    set_property(GLOBAL APPEND PROPERTY SHADY_SHADER_OUTPUTS "${params_OUTPUT_FILE_NAME}")
endfunction()

function(add_shader_target target_name shaders_path)
    file(GLOB_RECURSE shader_sources CONFIGURE_DEPENDS
        "${shaders_path}/*.comp"
        "${shaders_path}/*.frag"
        "${shaders_path}/*.geom"
        "${shaders_path}/*.tesc"
        "${shaders_path}/*.tese"
        "${shaders_path}/*.vert"
    )

    foreach(shader_source IN LISTS shader_sources)
        compile_shader(
            SOURCE_FILE "${shader_source}"
            OUTPUT_FILE_NAME "${shader_source}.spv"
        )
    endforeach()

    get_property(shader_outputs GLOBAL PROPERTY SHADY_SHADER_OUTPUTS)

    if (NOT shader_outputs)
        message(FATAL_ERROR "add_shader_target: no shaders found in ${shaders_path}")
    endif()

    add_custom_target(${target_name} DEPENDS ${shader_outputs})
endfunction()

set(SHADY_SHADERS_PATH "${PROJECT_SOURCE_DIR}/assets/shaders")

add_shader_target(shaders "${SHADY_SHADERS_PATH}")
add_dependencies(${PROJECT_NAME} shaders)
