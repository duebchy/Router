cmake_minimum_required(VERSION 3.12)
project(router LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.0)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3)
FetchContent_MakeAvailable(nlohmann_json)

include_directories(src bin)

add_subdirectory(src/cache)
add_subdirectory(src/api)
add_subdirectory(src/stations)
add_subdirectory(src/router)
add_subdirectory(bin)
enable_testing()
add_subdirectory(tests)

include(tests/coverage.cmake)
