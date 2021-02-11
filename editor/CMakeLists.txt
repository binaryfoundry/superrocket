cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME superrocket-editor)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(EXTERNAL_DEPS_DIR "lib")

project(${PROJECT_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCES
    "src/Math.cpp"
    "src/System.cpp"
    "src/Spline.cpp"
    "src/Main.cpp"
    "src/Drawing.cpp")

set(HEADERS
    "src/Math.hpp"
    "src/System.hpp"
    "src/Spline.hpp"
    "src/Main.hpp"
    "src/Drawing.hpp")

SOURCE_GROUP("Source" FILES ${SOURCES})
SOURCE_GROUP("Source" FILES ${HEADERS})

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/lib/glm)

include_directories(${PROJECT_SOURCE_DIR}/${EXTERNAL_DEPS_DIR}/sdl/win/include)
include_directories(${PROJECT_SOURCE_DIR}/${EXTERNAL_DEPS_DIR}/sdl_image/win/include)
include_directories(${PROJECT_SOURCE_DIR}/${EXTERNAL_DEPS_DIR}/sdl_gfx/win/include)
link_directories(${PROJECT_SOURCE_DIR}/${EXTERNAL_DEPS_DIR}/sdl/win/lib/x64)
link_directories(${PROJECT_SOURCE_DIR}/${EXTERNAL_DEPS_DIR}/sdl_image/win/lib/x64)
link_directories(${PROJECT_SOURCE_DIR}/${EXTERNAL_DEPS_DIR}/sdl_gfx/win/lib/x64)

set(LIBRARIES
    SDL2
    SDL2main
    SDL2_image
    SDL2_gfx)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS})

target_link_libraries(
    ${PROJECT_NAME}
    ${LIBRARIES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/lib/sdl/win/lib/x64/dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/lib/sdl_image/win/lib/x64"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/lib/sdl_gfx/win/lib/x64/dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
