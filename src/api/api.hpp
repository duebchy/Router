#pragma once

#include <expected>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

class ApiClientInterface {
 public:
  virtual ~ApiClientInterface() = default;

  virtual std::expected<nlohmann::json, std::string> Search(
      std::string_view from, std::string_view to, std::string_view date) = 0;
};
