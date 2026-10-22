cmake_minimum_required(VERSION 3.23)

project(dspsigvis LANGUAGES C)

find_package(glfw3 REQUIRED)

add_library(glad OBJECT "external/glad/src/gl.c")
target_include_directories(glad PUBLIC "external/glad/include")
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
unset(SOURCES)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME} PRIVATE glfw3 glad m)
