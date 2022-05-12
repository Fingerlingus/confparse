cmake_minimum_required(VERSION 3.16)
include(ExternalProject)

project(confparse)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

ExternalProject_Add(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
    BUILD_COMMAND cmake -E echo "Skipping build step."
    INSTALL_COMMAND cmake -E echo "Skipping install step."
    EXCLUDE_FROM_ALL TRUE
)

add_executable(test main.cpp)
add_dependencies(test fmt)
target_include_directories(test PRIVATE ${CMAKE_BINARY_DIR}/fmt-prefix/src/fmt/include)