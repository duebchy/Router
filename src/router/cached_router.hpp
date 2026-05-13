#pragma once

#include "cache/cache.hpp"
#include "router.hpp"

#include <expected>
#include <string>
#include <string_view>
#include <vector>

class CachedRouter final : public RouterInterface {
 public:
  CachedRouter(RouterInterface& inner, Cache& cache);

  std::expected<std::vector<Route>, std::string> FindRoutes(
      std::string_view from, std::string_view to,
      std::string_view date) override;

 private:
  RouterInterface& inner_;
  Cache& cache_;

  static nlohmann::json RoutesToJson(const std::vector<Route>& routes);
  static std::vector<Route> RoutesFromJson(const nlohmann::json& j);
};
