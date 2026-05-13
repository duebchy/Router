#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "api.hpp"
#include "http_session.hpp"

class YandexApiClient final : public ApiClientInterface {
  std::string api_key_;
  std::unique_ptr<HttpSessionInterface> owned_session_;
  HttpSessionInterface& session_;

 public:
  explicit YandexApiClient(std::string api_key);
  YandexApiClient(std::string api_key, HttpSessionInterface& session);

  std::expected<nlohmann::json, std::string> Search(
      std::string_view from, std::string_view to,
      std::string_view date) override;
};
