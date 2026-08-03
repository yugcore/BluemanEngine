cmake_minimum_required(VERSION 3.20)
project(engine_editor C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(WITH_DX12_BACKEND=1 ZEGFX_WITH_ASSIMP=0 IMGUI_DEFINE_MATH_OPERATORS)

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

# ImGui library with GLFW and DX12 backends
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
)

add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
)
target_link_libraries(imgui_lib PUBLIC glfw)

# Main Executable & ZeGFX Sources
file(GLOB SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/app/*.cpp"
    "src/theme/*.cpp"
    "src/widgets/*.cpp"
    "src/layout/*.cpp"
    "src/core/*.cpp"
    "src/engine/core/*.cpp"
    "src/engine/scene/*.cpp"
    "src/engine/assets/*.cpp"
    "src/render/*.cpp"
    "src/third_party/ImGuizmo/src/ImGuizmo.cpp"
    "src/panels/*.cpp"
    "src/panels/Chrome/*.cpp"
    "src/panels/Codebase/*.cpp"
    "src/panels/Run/*.cpp"
    "src/panels/ContentBrowser/*.cpp"
    "src/panels/Details/*.cpp"
    "src/panels/Outliner/*.cpp"
    "src/panels/OutputLog/*.cpp"
    "src/panels/RenderControlStrip/*.cpp"
    "src/panels/Viewport/*.cpp"
    "src/panels/ObjectPalette/*.cpp"
    "src/panels/MeshStudio/*.cpp"
    "src/panels/ShaderStudio/*.cpp"
    "src/panels/TextureViewer/*.cpp"
    "src/panels/ProjectWizard/*.cpp"
    "src/panels/ProjectSettings/*.cpp"
    "src/panels/ImportProgress/*.cpp"
    "src/panels/AssetCooking/*.cpp"

    # ZeGFX Graphics Engine Core Sources
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/zegfx.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/backend_factory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/default_backend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/headless_backend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/diagnostics.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/render_parameters.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/render_graph.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/render_world.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/renderer_spine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/shader_source.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/shader_compiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/shader_artifact.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/shader_reflection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/shader_variant.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/shader_library.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/pipeline_cache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/asset_importer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/asset_manager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/asset_registry.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/gltf_importer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/ufbx_importer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/vox_importer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/assimp_importer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/hot_reload.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/terrain_system.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/heightmap_importer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/binding_layout.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/platform_win32.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/dx12_pipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/dx12_dxr.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/zegfx_graph.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/zegfx_vox_translator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/zegfx_ufbx_translator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/zegfx_pipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/zegfx_factory.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/../src/dx12/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/pipeline/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/factories/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/translators/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/cooker/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/registry/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/bodies/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/broadphase/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/collision/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/materials/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/narrowphase/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/queries/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/physics/solver/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ufbx/ufbx.c"

    # Zelyn Language Scripting Runtime Sources
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Zelyn/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Zelyn/src/api/*.cpp"
)

add_executable(engine_editor ${SOURCES})
target_include_directories(engine_editor PRIVATE
    src
    src/third_party
    src/third_party/ImGuizmo
    src/third_party/ImGuizmo/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ufbx
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Zelyn/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Zelyn/include/zelyn
)

if (WIN32)
    target_link_libraries(engine_editor PRIVATE imgui_lib glfw d3d12 dxgi d3dcompiler dxguid ole32 winmm)
else()
    target_link_libraries(engine_editor PRIVATE imgui_lib glfw GL dl)
endif()
