add_library(stations station_repository.cpp)
target_link_libraries(stations cpr::cpr nlohmann_json::nlohmann_json)
