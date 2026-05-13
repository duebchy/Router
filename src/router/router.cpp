#include "router.hpp"

#include <expected>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "api/api.hpp"
#include "stations/station_repository.hpp"

Router::Router(ApiClientInterface& client, StationRepositoryInterface& stations)
    : client_(client), stations_(stations) {}

std::expected<std::vector<Route>, std::string> Router::FindRoutes(
    std::string_view from, std::string_view to, std::string_view date) {
  auto from_code = stations_.FindCode(from);
  if (!from_code)
    return std::unexpected(std::format("city not found: {}", from));

  auto to_code = stations_.FindCode(to);
  if (!to_code) return std::unexpected(std::format("city not found: {}", to));

  auto result = client_.Search(*from_code, *to_code, date);
  if (!result) return std::unexpected(result.error());

  return ParseRoutes(*result);
}

namespace {
std::string ExtractTime(const std::string& iso) {
  auto t = iso.find('T');
  if (t == std::string::npos || t + 6 > iso.size()) return iso;
  return iso.substr(t + 1, 5);
}

std::string CarrierTitle(const nlohmann::json& thread) {
  if (thread.contains("carrier") && thread["carrier"].contains("title"))
    return thread["carrier"]["title"].get<std::string>();
  if (thread.contains("title")) return thread["title"].get<std::string>();
  return {};
}
}  // namespace

std::vector<Route> Router::ParseRoutes(const nlohmann::json& j) {
  std::vector<Route> routes;

  for (auto& seg : j.value("segments", nlohmann::json::array())) {
    if (!seg.contains("departure") || !seg.contains("arrival")) continue;

    Route route;

    if (seg.value("has_transfers", false) && seg.contains("details")) {
      for (auto& detail : seg["details"]) {
        if (detail.value("is_transfer", false)) {
          std::string station;
          if (detail.contains("transfer_point") &&
              detail["transfer_point"].contains("title"))
            station = detail["transfer_point"]["title"].get<std::string>();
          route.transfer_stations.push_back(std::move(station));
        } else {
          if (!detail.contains("departure") || !detail.contains("arrival") ||
              !detail.contains("thread"))
            continue;
          route.segments.push_back(Segment{
              .carrier      = CarrierTitle(detail["thread"]),
              .departure_at = ExtractTime(detail["departure"].get<std::string>()),
              .arrives_at   = ExtractTime(detail["arrival"].get<std::string>()),
          });
        }
      }
    } else {
      if (!seg.contains("thread")) continue;
      route.segments.push_back(Segment{
          .carrier      = CarrierTitle(seg["thread"]),
          .departure_at = ExtractTime(seg["departure"].get<std::string>()),
          .arrives_at   = ExtractTime(seg["arrival"].get<std::string>()),
      });
    }

    if (!route.segments.empty())
      routes.push_back(std::move(route));
  }

  return routes;
}
