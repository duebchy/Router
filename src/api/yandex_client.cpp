#include "yandex_client.hpp"

#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace {
constexpr std::string_view kBaseUrl = "https://api.rasp.yandex.net/v3.0/search/";
}

YandexApiClient::YandexApiClient(std::string api_key)
    : api_key_(std::move(api_key)),
      owned_session_(std::make_unique<CprHttpSession>(kBaseUrl)),
      session_(*owned_session_) {}

YandexApiClient::YandexApiClient(std::string api_key, HttpSessionInterface& session)
    : api_key_(std::move(api_key)), session_(session) {}

std::expected<nlohmann::json, std::string> YandexApiClient::Search(
    std::string_view from, std::string_view to, std::string_view date) {
  auto response = session_.get({
      {"from",      std::string(from)},
      {"to",        std::string(to)},
      {"date",      std::string(date)},
      {"apikey",    api_key_},
      {"transfers", "true"},
      {"limit",     "1000"},
  });

  if (response.has_error)
    return std::unexpected(response.error_message);

  switch (response.status_code) {
    case 200:
      break;
    case 401:
      return std::unexpected("wrong API key");
    case 429:
      return std::unexpected("reached limit of requests");
    default:
      return std::unexpected(std::format("HTTP {}", response.status_code));
  }

  try {
    return nlohmann::json::parse(response.text);
  } catch (const nlohmann::json::exception& e) {
    return std::unexpected(std::format("json parse error: {}", e.what()));
  }
}
