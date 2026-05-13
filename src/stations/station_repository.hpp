#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

class StationRepositoryInterface {
 public:
  virtual ~StationRepositoryInterface() = default;

  virtual std::optional<std::string> FindCode(std::string_view city_name) = 0;
};

class StationRepository final : public StationRepositoryInterface {
  std::unordered_map<std::string, std::string> codes_;

 public:
  StationRepository(std::filesystem::path stations_file,
                    std::string_view api_key);

  std::optional<std::string> FindCode(std::string_view city_name) override;
};
