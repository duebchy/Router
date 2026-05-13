#include "cached_router.hpp"

#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

CachedRouter::CachedRouter(RouterInterface& inner, Cache& cache)
    : inner_(inner), cache_(cache) {}

std::expected<std::vector<Route>, std::string> CachedRouter::FindRoutes(
    std::string_view from, std::string_view to, std::string_view date) {
  auto key = std::format("{}|{}|{}", from, to, date);

  if (auto cached = cache_.Get(key)) return RoutesFromJson(*cached);

  auto result = inner_.FindRoutes(from, to, date);
  if (result) cache_.Put(key, RoutesToJson(*result), cache_.RouteTtl(date));

  return result;
}

nlohmann::json CachedRouter::RoutesToJson(const std::vector<Route>& routes) {
  nlohmann::json j = nlohmann::json::array();
  for (auto& route : routes) {
    nlohmann::json segs = nlohmann::json::array();
    for (auto& seg : route.segments)
      segs.push_back(
          {{"c", seg.carrier}, {"d", seg.departure_at}, {"a", seg.arrives_at}});
    j.push_back({{"s", segs}, {"t", route.transfer_stations}});
  }
  return j;
}

std::vector<Route> CachedRouter::RoutesFromJson(const nlohmann::json& j) {
  std::vector<Route> routes;
  for (auto& entry : j) {
    Route route;
    for (auto& seg : entry["s"])
      route.segments.push_back(Segment{
          .carrier = seg["c"].get<std::string>(),
          .departure_at = seg["d"].get<std::string>(),
          .arrives_at = seg["a"].get<std::string>(),
      });
    if (entry.contains("t"))
      route.transfer_stations = entry["t"].get<std::vector<std::string>>();
    routes.push_back(std::move(route));
  }
  return routes;
}
