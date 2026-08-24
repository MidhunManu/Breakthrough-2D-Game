cmake_minimum_required(VERSION 4.3)
project(trench_invasion)

set(CMAKE_CXX_STANDARD 23)
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
add_executable(trench_invasion main.cpp)
target_link_libraries(trench_invasion PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)