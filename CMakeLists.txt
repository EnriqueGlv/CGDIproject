cmake_minimum_required(VERSION 3.16.3)

project(SeamCarving LANGUAGES C CXX)

#=================== SDL3 ===================

set(SDL3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL)
add_subdirectory(${SDL3_DIR})


#=================== IMGUI ===================

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)
add_library(IMGUI STATIC)

target_sources( IMGUI
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
                )

target_include_directories( IMGUI
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            PUBLIC ${SDL3_DIR}/include
                            )


find_package(OpenGL REQUIRED)
target_link_libraries(IMGUI PUBLIC ${OPENGL_LIBRARIES})

target_link_libraries(IMGUI PUBLIC SDL3-shared ${CMAKE_DL_LIBS})



#=================== EXAMPLE ===================

set(CMAKE_SOURCE_DIR "src")
set(CMAKE_BINARY_DIR "bin")

add_executable(example)
target_sources(example 
                PUBLIC 
                    ${CMAKE_SOURCE_DIR}/main.cpp
                    ${CMAKE_SOURCE_DIR}/seamCarving.cpp
                )
target_link_libraries(example IMGUI)
set_target_properties(example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
