cmake_minimum_required(VERSION 3.21)
project(matf-rg-engine)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++23" COMPILER_SUPPORTS_CXX23)
if (NOT COMPILER_SUPPORTS_CXX23)
    message(FATAL_ERROR "The compiler does not support C++23.")
endif()

file(GLOB engine-sources src/*.cpp)
file(GLOB engine-headers include/engine/*.hpp)

################ Libs ################
add_subdirectory(libs/spdlog EXCLUDE_FROM_ALL)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE SPDLOG_DEBUG_ON SPDLOG_TRACE_ON RG_ENGINE_TRACE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(ASSIMP REQUIRED)

add_subdirectory(libs/glfw EXCLUDE_FROM_ALL)
add_subdirectory(libs/glad EXCLUDE_FROM_ALL)
add_subdirectory(libs/json EXCLUDE_FROM_ALL)
add_subdirectory(libs/stb EXCLUDE_FROM_ALL)
add_subdirectory(libs/imgui EXCLUDE_FROM_ALL)
add_subdirectory(libs/glm EXCLUDE_FROM_ALL)

################ Embedded engine shaders ################
set(ENGINE_SHADERS depth depth_spot blur bloom_final light_debug)
set(ENGINE_SHADER_HEADERS "")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)

foreach(shader ${ENGINE_SHADERS})
    set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/${shader}.glsl)
    set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/${shader}_glsl.h)
    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND xxd -i ${shader}.glsl > ${OUTPUT}
        DEPENDS ${INPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders
    )
    list(APPEND ENGINE_SHADER_HEADERS ${OUTPUT})
endforeach()

add_custom_target(engine-shaders DEPENDS ${ENGINE_SHADER_HEADERS})

add_library(${PROJECT_NAME} ${engine-sources} ${engine-headers})
add_dependencies(${PROJECT_NAME} engine-shaders)
target_include_directories(${PROJECT_NAME} PUBLIC include/ PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw assimp ${ASSIMP_LIBRARIES} stb
        PUBLIC glm::glm-header-only spdlog::spdlog imgui json)
