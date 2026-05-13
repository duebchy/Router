add_library(router_lib router.cpp cached_router.cpp)
target_link_libraries(router_lib cache nlohmann_json::nlohmann_json)
