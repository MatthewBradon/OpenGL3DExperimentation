cmake_minimum_required(VERSION 3.15)
project(OpenGL3DExperimentation)
set(CMAKE_CXX_STANDARD 17)

# Packages
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(GLUT REQUIRED)
find_package(Stb REQUIRED)

# OpenGL3DExperimentation executable
add_executable(OpenGL3DExperimentation
    src/main.cpp
)

# Set output directory for all configurations
set_target_properties(OpenGL3DExperimentation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
)

target_include_directories(OpenGL3DExperimentation PRIVATE
    ${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-windows/include
)

# Link libraries
target_link_libraries(OpenGL3DExperimentation PRIVATE
    glad::glad
    glfw
    imgui::imgui
    GLUT::GLUT
)