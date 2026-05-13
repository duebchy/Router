add_executable(router main.cpp repl.cpp)
target_link_libraries(router api router_lib stations cache)
