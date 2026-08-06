cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
project(myproj)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# glad built as library
add_library(glad STATIC vendored/glad/src/glad.c)
target_include_directories(glad PUBLIC vendored/glad/include)

# Create your game executable target as usual
# WIN32 option is ignored on macOS and Linux
add_executable(myproj WIN32 src/main.cpp)

# glm (head-only)
target_include_directories(myproj PRIVATE vendored/glm)

# Link to SDL3 and glad libraries
target_link_libraries(myproj PRIVATE SDL3::SDL3 glad)
