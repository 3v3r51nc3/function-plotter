cmake_minimum_required(VERSION 3.15)
project(function-plotter VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(OpenGL REQUIRED)

# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)

# Add imgui as a library (assumes imgui is in external/imgui or as submodule)
# You'll need to download imgui and place it in external/imgui directory
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
if(NOT EXISTS ${IMGUI_DIR})
    message(FATAL_ERROR "ImGui not found at ${IMGUI_DIR}. Please download ImGui from https://github.com/ocornut/imgui")
endif()

# Add GLFW (assumes glfw is in external/glfw or as submodule)
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
if(NOT EXISTS ${GLFW_DIR})
    message(FATAL_ERROR "GLFW not found at ${GLFW_DIR}. Please download GLFW from https://github.com/glfw/glfw")
endif()

add_subdirectory(${GLFW_DIR} EXCLUDE_FROM_ALL)

# Add exprtk (header-only, assumes it's in external/exprtk)
set(EXPRTK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/exprtk)
if(NOT EXISTS ${EXPRTK_DIR})
    message(WARNING "ExprTk not found at ${EXPRTK_DIR}. Please download ExprTk from https://github.com/ArashPartow/exprtk")
endif()

# ImGui sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Application sources
set(APP_SOURCES
    main.cpp
    src/core/App.cpp
    src/core/Config.cpp
    src/ui/GuiManager.cpp
    src/render/RendererGL.cpp
    src/render/Scene.cpp
)

# Application headers
set(APP_HEADERS
    src/Animation.h
    src/core/App.h
    src/core/Config.h
    src/ui/GuiManager.h
    src/render/RendererGL.h
    src/render/Scene.h
)

# Create executable
add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_HEADERS} ${IMGUI_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${EXPRTK_DIR}
    ${OPENGL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Set console subsystem for debugging, or WINDOWS for release
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE FALSE
        )
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()
elseif(APPLE)
    # macOS-specific settings
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
elseif(UNIX)
    # Linux-specific settings
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${X11_LIBRARIES}
        ${CMAKE_DL_LIBS}
        Threads::Threads
    )
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES config.ini config.txt DESTINATION bin)
