#pragma once

#include <gmock/gmock.h>

#include "api/api.hpp"
#include "api/http_session.hpp"
#include "router/router.hpp"
#include "stations/station_repository.hpp"

class MockHttpSession : public HttpSessionInterface {
 public:
  using Params = std::unordered_map<std::string, std::string>;
  MOCK_METHOD(HttpResponse, get, (const Params& params), (override));
};

class MockApiClient : public ApiClientInterface {
public:
    MOCK_METHOD((std::expected<nlohmann::json, std::string>), Search,
                (std::string_view from, std::string_view to, std::string_view date), (override));
};

class MockStationRepository : public StationRepositoryInterface {
public:
    MOCK_METHOD(std::optional<std::string>, FindCode, (std::string_view city_name), (override));
};

class MockRouter : public RouterInterface {
public:
    MOCK_METHOD((std::expected<std::vector<Route>, std::string>), FindRoutes,
                (std::string_view from, std::string_view to, std::string_view date), (override));
};
