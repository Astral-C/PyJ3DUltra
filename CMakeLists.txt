cmake_minimum_required(VERSION 3.4...3.18)
project(J3DUltraPy)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Python version to build for
set(PYBIND11_PYTHON_VERSION "3.11")
set(PYTHON_EXECUTABLE /usr/bin/python3.11)

add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_subdirectory(pybind11)
add_subdirectory(J3DUltra)

pybind11_add_module(J3DUltraPy src/main.cpp)

target_include_directories(J3DUltraPy PUBLIC J3DUltra/include J3DUltra/lib/bStream pybind11/pybind11)

target_link_libraries(J3DUltraPy PUBLIC j3dultra pybind11::embed)

# Build as 'J3DUltraPy' but rename to 'J3DUltra' afterwards to avoid naming conflicts during the build.
set_target_properties(J3DUltraPy PROPERTIES OUTPUT_NAME J3DUltra)
