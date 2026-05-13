#pragma once

#include <chrono>
#include <list>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

class Cache {
 public:
  explicit Cache(std::size_t max_size = 200);

  std::optional<nlohmann::json> Get(std::string_view key);
  void Put(std::string_view key, nlohmann::json data, std::chrono::seconds ttl);

  std::vector<std::string> Keys() const;
  std::chrono::seconds RouteTtl(std::string_view date);

 private:
  struct Entry {
    nlohmann::json data;
    std::chrono::system_clock::time_point expires_at;
    std::list<std::string>::iterator lru_it;
  };

  void Evict();

  std::size_t max_size_;
  std::unordered_map<std::string, Entry> index_;
  std::list<std::string> lru_;
};
