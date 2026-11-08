cmake_minimum_required(VERSION 3.20)
project(fileio_bench CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# all executables next to each other in build/ (build/Release/ with Visual Studio)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(common)
add_subdirectory(01_apis)
add_subdirectory(02_buffered)
