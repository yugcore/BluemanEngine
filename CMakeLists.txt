cmake_minimum_required(VERSION 3.20)
project(engine_editor C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# ImGui (docking branch)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# ImGui library with GLFW and OpenGL3 backends
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
)
target_link_libraries(imgui_lib PUBLIC glfw)

# Main Executable
file(GLOB SOURCES
    "src/*.cpp"
    "src/app/*.cpp"
    "src/theme/*.cpp"
    "src/widgets/*.cpp"
    "src/layout/*.cpp"
    "src/core/*.cpp"
    "src/render/*.cpp"
    "src/third_party/glad/*.c"
    "src/third_party/ImGuizmo/*.cpp"
    "src/panels/Chrome/*.cpp"
    "src/panels/ContentBrowser/*.cpp"
    "src/panels/Details/*.cpp"
    "src/panels/Outliner/*.cpp"
    "src/panels/OutputLog/*.cpp"
    "src/panels/RenderControlStrip/*.cpp"
    "src/panels/Viewport/*.cpp"
)

add_executable(engine_editor ${SOURCES})
target_include_directories(engine_editor PRIVATE
    src
    src/third_party
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

if (WIN32)
    target_link_libraries(engine_editor PRIVATE imgui_lib glfw opengl32)
else()
    target_link_libraries(engine_editor PRIVATE imgui_lib glfw GL dl)
endif()
