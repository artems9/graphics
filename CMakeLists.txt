cmake_minimum_required(VERSION 3.16)
project(myproj)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Create your game executable target as usual
# WIN32 option is ignored on macOS and Linux
add_executable(myproj WIN32 src/main.cpp)

# Link to the actual SDL3 library.
target_link_libraries(myproj PRIVATE SDL3::SDL3)
