add_library(api yandex_client.cpp)
target_link_libraries(api cpr::cpr nlohmann_json::nlohmann_json)
