# CMakeList.txt : CMake project for omcmeas, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)


project("threed" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

include(FetchContent)

find_package(raylib QUIET)
if (NOT raylib_FOUND)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        )
    FetchContent_MakeAvailable(raylib)
endif()

FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.2
)

FetchContent_MakeAvailable(cli11_proj)

FetchContent_Declare(
    nbt
    GIT_REPOSITORY https://github.com/awegsche/nbt.git
    GIT_TAG main
    )
FetchContent_MakeAvailable(nbt)

add_executable(threed
    "src/main.cpp"
    )

target_link_libraries(threed
    raylib
    spdlog::spdlog
    assimp::assimp
    CLI11::CLI11
    nbtlib
    )
