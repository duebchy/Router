#include <gtest/gtest.h>

#include <chrono>

#include "cache/cache.hpp"

TEST(Cache, MissOnEmpty) {
  Cache cache;
  EXPECT_FALSE(cache.Get("key").has_value());
}

TEST(Cache, HitAfterPut) {
  Cache cache;
  cache.Put("key", nlohmann::json{{"a", 1}}, std::chrono::hours(1));
  auto result = cache.Get("key");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ((*result)["a"], 1);
}

TEST(Cache, ExpiredEntryReturnsNullopt) {
  Cache cache;
  cache.Put("key", nlohmann::json{{"a", 1}}, std::chrono::seconds(-1));
  EXPECT_FALSE(cache.Get("key").has_value());
}

TEST(Cache, EvictsLruWhenFull) {
  Cache cache(2);
  cache.Put("a", nlohmann::json(1), std::chrono::hours(1));
  cache.Put("b", nlohmann::json(2), std::chrono::hours(1));
  cache.Put("c", nlohmann::json(3), std::chrono::hours(1));

  EXPECT_FALSE(cache.Get("a").has_value());
  EXPECT_TRUE(cache.Get("b").has_value());
  EXPECT_TRUE(cache.Get("c").has_value());
}

TEST(Cache, UpdateExistingKey) {
  Cache cache;
  cache.Put("key", nlohmann::json(1), std::chrono::hours(1));
  cache.Put("key", nlohmann::json(2), std::chrono::hours(1));
  auto result = cache.Get("key");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 2);
}

TEST(Cache, RouteTtlPastDate) {
  Cache cache;
  auto ttl = cache.RouteTtl("2020-01-01");
  EXPECT_EQ(ttl, std::chrono::days(30));
}

TEST(Cache, RouteTtlFutureDate) {
  Cache cache;
  auto ttl = cache.RouteTtl("2099-12-31");
  EXPECT_EQ(ttl, std::chrono::hours(6));
}

TEST(Cache, RouteTtlInvalidDate) {
  Cache cache;
  auto ttl = cache.RouteTtl("not-a-date");
  EXPECT_EQ(ttl, std::chrono::hours(6));
}

TEST(Cache, GetRefreshesLruOrder) {
  Cache cache(2);
  cache.Put("a", nlohmann::json(1), std::chrono::hours(1));
  cache.Put("b", nlohmann::json(2), std::chrono::hours(1));
  cache.Get("a");
  cache.Put("c", nlohmann::json(3), std::chrono::hours(1));
  EXPECT_FALSE(cache.Get("b").has_value());
  EXPECT_TRUE(cache.Get("a").has_value());
  EXPECT_TRUE(cache.Get("c").has_value());
}
