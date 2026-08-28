cmake_minimum_required(VERSION 4.2)

project(trench_invasion)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)

include(FetchContent)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glm)

add_executable(trench_invasion
        main.cpp
        Timer.h
        Animation.h
        GameObject.h
)

target_link_libraries(trench_invasion
        PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        glm::glm
)

# target_compile_definitions(trench_invasion PRIVATE DEBUG)
