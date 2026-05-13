include(FetchContent)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2)
FetchContent_MakeAvailable(googletest)

add_executable(tests
    test_cache.cpp
    test_router.cpp
    test_cached_router.cpp
    test_env.cpp
    test_station_repository.cpp
    test_yandex_client.cpp
)

target_link_libraries(tests cache router_lib api stations GTest::gtest_main GTest::gmock)

include(GoogleTest)
gtest_discover_tests(tests)
