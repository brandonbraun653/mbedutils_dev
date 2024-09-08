cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER_WORKS)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(MbedutilsDevelopment C CXX)
enable_testing()

add_subdirectory(mbedutils)
add_subdirectory(tests)