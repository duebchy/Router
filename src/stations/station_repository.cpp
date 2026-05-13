#include "station_repository.hpp"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace {
constexpr std::string_view kStationsListUrl =
    "https://api.rasp.yandex.net/v3.0/stations_list/";

void DownloadStations(std::string_view api_key,
                       const std::filesystem::path& out_file,
                       std::unordered_map<std::string, std::string>& codes) {
    std::cout << "Downloading stations list...\n";

    auto response = cpr::Get(
        cpr::Url{std::string(kStationsListUrl)},
        cpr::Parameters{{"apikey", std::string(api_key)},
                        {"lang", "ru_RU"},
                        {"format", "json"}});

    if (response.error || response.status_code != 200) {
        throw std::runtime_error("failed to download stations list");
    }

    auto j = nlohmann::json::parse(response.text);
    nlohmann::json simplified;

    for (auto& country : j["countries"]) {
        for (auto& region : country["regions"]) {
            for (auto& settlement : region["settlements"]) {
                auto title = settlement["title"].get<std::string>();
                auto code  = settlement["codes"].value("yandex_code", "");
                if (!title.empty() && !code.empty()) {
                    codes[title] = code;
                    simplified[title] = code;
                }
            }
        }
    }

    std::ofstream f(out_file);
    if (f.is_open()) f << simplified.dump();
}

void LoadStations(const std::filesystem::path& file,
                   std::unordered_map<std::string, std::string>& codes) {
    std::ifstream f(file);
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(f);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::format("failed to parse stations file: {}", e.what()));
    }
    for (auto& [name, code] : j.items()) {
        codes[name] = code.get<std::string>();
    }
}

bool IsExpired(const std::filesystem::path& file) {
    auto ftime = std::filesystem::last_write_time(file);
    auto age = std::filesystem::file_time_type::clock::now() - ftime;
    return age > std::chrono::days(30);
}
} // namespace

StationRepository::StationRepository(std::filesystem::path stations_file,
                                     std::string_view api_key) {
    if (std::filesystem::exists(stations_file) && !IsExpired(stations_file)) {
        LoadStations(stations_file, codes_);
    } else {
        DownloadStations(api_key, stations_file, codes_);
    }
}

std::optional<std::string> StationRepository::FindCode(std::string_view city_name) {
    auto it = codes_.find(std::string(city_name));
    if (it == codes_.end()) return std::nullopt;
    return it->second;
}
