cmake_minimum_required(VERSION 3.25)

project(
    LearnWebGPU
    LANGUAGES CXX C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(ext/sdl2)
add_subdirectory(ext/webgpu)
add_subdirectory(ext/sdl2webgpu)
add_subdirectory(ext/glm)
add_subdirectory(ext/tinygltf-2.9.3)

add_definitions(-std=c++20)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE WEBGPU_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${CMAKE_PROJECT_NAME} ${WEBGPU_SRC_FILES})

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_LEFT_HANDED)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ext)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL2::SDL2 webgpu sdl2webgpu glm)

target_copy_webgpu_binaries(${CMAKE_PROJECT_NAME})

if (EMSCRIPTEN)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -sASYNCIFY)
else()
    set(SHADER_SOURCE "${CMAKE_SOURCE_DIR}/shaders/colorpass.wgsl")
    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND
          ${CMAKE_COMMAND} -E copy_if_different
          ${SHADER_SOURCE}
          $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/shaders/colorpass.wgsl
      )

    set(MODEL_SOURCE "${CMAKE_SOURCE_DIR}/models/DamagedHelmet.glb")
    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND
          ${CMAKE_COMMAND} -E copy_if_different
          ${SHADER_SOURCE}
          $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/models/DamagedHelmet.glb
      )
endif()