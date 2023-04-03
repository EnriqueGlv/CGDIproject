if (TARGET imgui)
  return()
endif()

include(FetchContent)

message(STATUS "Fetching imgui")

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_SHALLOW    TRUE
    )
FetchContent_MakeAvailable(imgui)
