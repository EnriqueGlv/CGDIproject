project(CGDI-project)

CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(${PROJECT_SOURCE_DIR}/deps)
message(STATUS "Deps: ${PROJECT_SOURCE_DIR}/deps")

############################################################
include(imgui)

add_subdirectory(src)
############################################################
