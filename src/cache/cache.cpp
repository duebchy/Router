#include "cache.hpp"

#include <chrono>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

Cache::Cache(std::size_t max_size) : max_size_(max_size) {}

std::optional<nlohmann::json> Cache::Get(std::string_view key) {
  auto it = index_.find(std::string(key));
  if (it == index_.end()) return std::nullopt;

  if (std::chrono::system_clock::now() > it->second.expires_at) {
    lru_.erase(it->second.lru_it);
    index_.erase(it);
    return std::nullopt;
  }

  lru_.erase(it->second.lru_it);
  lru_.push_front(std::string(key));
  it->second.lru_it = lru_.begin();

  return it->second.data;
}

void Cache::Put(std::string_view key, nlohmann::json data,
                std::chrono::seconds ttl) {
  auto skey = std::string(key);

  if (auto it = index_.find(skey); it != index_.end()) {
    lru_.erase(it->second.lru_it);
    index_.erase(it);
  }

  Evict();

  lru_.push_front(skey);
  index_[skey] = Entry{.data = std::move(data),
                       .expires_at = std::chrono::system_clock::now() + ttl,
                       .lru_it = lru_.begin()};
}

std::vector<std::string> Cache::Keys() const {
  auto now = std::chrono::system_clock::now();
  std::vector<std::string> result;
  for (auto& key : lru_) {
    auto it = index_.find(key);
    if (it != index_.end() && now <= it->second.expires_at)
      result.push_back(key);
  }
  return result;
}

std::chrono::seconds Cache::RouteTtl(std::string_view date) {
  int y = 0, m = 0, d = 0;
  if (std::sscanf(date.data(), "%d-%d-%d", &y, &m, &d) != 3)
    return std::chrono::hours(6);

  auto route_day = std::chrono::sys_days{
      std::chrono::year(y) / std::chrono::month(m) / std::chrono::day(d)};
  auto today = std::chrono::floor<std::chrono::days>(
      std::chrono::system_clock::now());

  if (route_day < today)
    return std::chrono::days(30);
  return std::chrono::hours(6);
}

void Cache::Evict() {
  if (index_.size() < max_size_) return;

  auto oldest = lru_.back();
  index_.erase(oldest);
  lru_.pop_back();
}
