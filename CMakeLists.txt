cmake_minimum_required(VERSION 3.4...3.18)
project(J3DUltraPy)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Python version to build for
# set(PYBIND11_PYTHON_VERSION "3.11")

add_subdirectory(pybind11)
add_subdirectory(J3DUltra)

pybind11_add_module(J3DUltra src/main.cpp)

target_include_directories(J3DUltra PUBLIC J3DUltra/include J3DUltra/lib/bStream pybind11/pybind11)

target_link_libraries(J3DUltra PUBLIC j3dultra pybind11::embed)