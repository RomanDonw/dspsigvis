cmake_minimum_required(VERSION 3.23)

project(dspsigvis LANGUAGES C)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
unset(SOURCES)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
