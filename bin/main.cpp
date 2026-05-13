#include <iostream>
#include <stdexcept>
#include <string>

#include "api/yandex_client.hpp"
#include "env.hpp"
#include "repl.hpp"
#include "router/cached_router.hpp"
#include "router/router.hpp"
#include "stations/station_repository.hpp"

int main() {
  try {
    LoadDotenv();

    auto api_key = GetEnv("YANDEX_API_KEY");
    if (!api_key) throw std::runtime_error("YANDEX_API_KEY is not set");

    const auto stations_file =
        GetEnv("STATIONS_FILE").value_or("stations.json");

    YandexApiClient yandex_client(std::string{*api_key});
    StationRepository station_repo(stations_file, *api_key);
    Router router(yandex_client, station_repo);
    Cache cache;
    CachedRouter cached_router(router, cache);

    Run(cached_router, cache);
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
}
