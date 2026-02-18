cmake_minimum_required(VERSION 3.20)
project(gw2-viewer VERSION 0.1.0 LANGUAGES C CXX)

# =========================
# C++20 Strict
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF)

# =========================
# Recursively collect sources
# =========================
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

# =========================
# ImGui sources (docking branch)
# =========================
set(IMGUI_DIR external/imgui-docking)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# =========================
# GLAD
# =========================
add_library(glad STATIC
    external/glad33extcore/src/glad.c
)
set_source_files_properties(
    external/glad33extcore/src/glad.c
    PROPERTIES LANGUAGE C
)
target_include_directories(glad PUBLIC
    external/glad33extcore/include
)

# =========================
# GLFW (IMPORTED)
# =========================
add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
    IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/external/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a
)

# =========================
# Executable
# =========================
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${IMGUI_SOURCES}
)

# =========================
# Include directories
# =========================
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    external/imgui-docking
    external/imgui-docking/backends
    external/glfw-3.4.bin.WIN64/include
    external/glm-1.0.3
    external/ImGuizmo-master
    external/stb-master
    external/half-therock-7.11/include
    external/nlohmann
    external/dr_libs-master
    external/miniaudio-0.11.24
)

# =========================
# Compile definitions
# =========================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLFW_INCLUDE_NONE
    IMGUI_IMPL_OPENGL_LOADER_GLAD
    # Prevent stb_image from being defined in multiple TUs
    # Define it only in PreviewPanel.cpp via #define STB_IMAGE_IMPLEMENTATION
)

# =========================
# Link
# =========================
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glad
        glfw3
        opengl32
        gdi32
        user32
        shell32
        comdlg32    # For GetOpenFileName WinAPI
        dwmapi
)

file(GLOB_RECURSE PROJECT_ASSETS
    ${CMAKE_SOURCE_DIR}/assets/*
)

foreach(asset ${PROJECT_ASSETS})
    file(RELATIVE_PATH rel ${CMAKE_SOURCE_DIR}/assets ${asset})
    configure_file(${asset}
        ${CMAKE_BINARY_DIR}/assets/${rel}
        COPYONLY)
endforeach()
