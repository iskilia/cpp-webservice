
# ============================================================================
# CMakeLists.txt - Build configuration for C++20 web service
# ============================================================================
cmake_minimum_required(VERSION 3.20)
project(cpp-web-service VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Include FetchContent for dependencies
include(FetchContent)

# Fetch httplib
FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

# Fetch nlohmann/json
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Create executable
add_executable(cpp-web-service src/main.cpp)

# Link libraries
target_link_libraries(cpp-web-service
        PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
        Threads::Threads
)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(cpp-web-service PRIVATE -fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(cpp-web-service PRIVATE -stdlib=libc++)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(cpp-web-service PRIVATE /std:c++20)
endif()

# Installation
install(TARGETS cpp-web-service
        RUNTIME DESTINATION bin
)