#pragma once

#include <expected>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "api/api.hpp"
#include "stations/station_repository.hpp"

struct Segment {
  std::string carrier;
  std::string departure_at;
  std::string arrives_at;
};

struct Route {
  std::vector<Segment> segments;
  std::vector<std::string> transfer_stations;
};

class RouterInterface {
 public:
  virtual ~RouterInterface() = default;

  virtual std::expected<std::vector<Route>, std::string> FindRoutes(
      std::string_view from, std::string_view to, std::string_view date) = 0;
};

class Router final : public RouterInterface {
 public:
  Router(ApiClientInterface& client, StationRepositoryInterface& stations);

  std::expected<std::vector<Route>, std::string> FindRoutes(
      std::string_view from, std::string_view to,
      std::string_view date) override;

 private:
  ApiClientInterface& client_;
  StationRepositoryInterface& stations_;

  static std::vector<Route> ParseRoutes(const nlohmann::json& j);
};
