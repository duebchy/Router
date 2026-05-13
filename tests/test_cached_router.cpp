#include <gtest/gtest.h>

#include <expected>
#include <string>
#include <vector>

#include "mocks.hpp"
#include "router/cached_router.hpp"

using ::testing::Return;

static std::vector<Route> make_routes() {
  return {Route{
      .segments = {Segment{
          .carrier = "ФПК", .departure_at = "10:00", .arrives_at = "18:00"}}}};
}

TEST(CachedRouter, CallsInnerOnMiss) {
  MockRouter inner;
  Cache cache;
  EXPECT_CALL(inner, FindRoutes("А", "Б", "2026-03-25"))
      .WillOnce(Return(make_routes()));

  CachedRouter router(inner, cache);
  auto result = router.FindRoutes("А", "Б", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ((*result)[0].segments[0].carrier, "ФПК");
}

TEST(CachedRouter, DoesNotCallInnerOnHit) {
  MockRouter inner;
  Cache cache;
  EXPECT_CALL(inner, FindRoutes("А", "Б", "2026-03-25"))
      .WillOnce(Return(make_routes()));

  CachedRouter router(inner, cache);
  router.FindRoutes("А", "Б", "2026-03-25");
  auto result = router.FindRoutes("А", "Б", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ((*result)[0].segments[0].carrier, "ФПК");
}

TEST(CachedRouter, PropagatesError) {
  MockRouter inner;
  Cache cache;
  EXPECT_CALL(inner, FindRoutes("А", "Б", "2026-03-25"))
      .WillOnce(Return(std::unexpected<std::string>("city not found: А")));

  CachedRouter router(inner, cache);
  auto result = router.FindRoutes("А", "Б", "2026-03-25");
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "city not found: А");
}
