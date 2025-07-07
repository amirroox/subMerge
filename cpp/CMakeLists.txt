cmake_minimum_required(VERSION 3.30)
project(subMerge)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(subMerge main.cpp)

target_link_libraries(subMerge PRIVATE nlohmann_json::nlohmann_json)