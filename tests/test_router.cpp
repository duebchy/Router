#include <gtest/gtest.h>

#include <expected>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "mocks.hpp"
#include "router/router.hpp"

using ::testing::Return;

static nlohmann::json make_segment(const std::string& carrier,
                                   const std::string& dep,
                                   const std::string& arr) {
  return {{"thread", {{"carrier", {{"title", carrier}}}}},
          {"departure", dep},
          {"arrival", arr},
          {"has_transfers", false}};
}

TEST(Router, CityNotFound) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Unknown")).WillOnce(Return(std::nullopt));

  Router router(api, stations);
  auto result = router.FindRoutes("Unknown", "Москва", "2026-03-25");
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "city not found: Unknown");
}

TEST(Router, ApiError) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Санкт-Петербург"))
      .WillOnce(Return(std::string("c2")));
  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));
  EXPECT_CALL(api, Search("c2", "c213", "2026-03-25"))
      .WillOnce(Return(std::unexpected<std::string>("HTTP 429")));

  Router router(api, stations);
  auto result = router.FindRoutes("Санкт-Петербург", "Москва", "2026-03-25");
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "HTTP 429");
}

TEST(Router, ParsesDirectRoute) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Санкт-Петербург"))
      .WillOnce(Return(std::string("c2")));
  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));

  nlohmann::json response = {
      {"segments",
       {make_segment("Сапсан", "2026-03-25T10:00:00+03:00",
                     "2026-03-25T14:30:00+03:00")}}};
  EXPECT_CALL(api, Search("c2", "c213", "2026-03-25"))
      .WillOnce(Return(response));

  Router router(api, stations);
  auto result = router.FindRoutes("Санкт-Петербург", "Москва", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 1);
  EXPECT_EQ((*result)[0].segments[0].carrier, "Сапсан");
  EXPECT_EQ((*result)[0].segments[0].departure_at, "10:00");
  EXPECT_EQ((*result)[0].segments[0].arrives_at, "14:30");
}

TEST(Router, EmptySegments) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Санкт-Петербург"))
      .WillOnce(Return(std::string("c2")));
  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));
  EXPECT_CALL(api, Search("c2", "c213", "2026-03-25"))
      .WillOnce(Return(nlohmann::json{{"segments", nlohmann::json::array()}}));

  Router router(api, stations);
  auto result = router.FindRoutes("Санкт-Петербург", "Москва", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST(Router, SkipsSegmentWithoutDeparture) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Санкт-Петербург"))
      .WillOnce(Return(std::string("c2")));
  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));

  nlohmann::json response = {
      {"segments",
       {{{"arrival", "2026-03-25T14:00:00+03:00"},
         {"thread", {{"title", "X"}}}},
        make_segment("Сапсан", "2026-03-25T10:00:00+03:00",
                     "2026-03-25T14:30:00+03:00")}}};
  EXPECT_CALL(api, Search("c2", "c213", "2026-03-25"))
      .WillOnce(Return(response));

  Router router(api, stations);
  auto result = router.FindRoutes("Санкт-Петербург", "Москва", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->size(), 1);
}

TEST(Router, CarrierTitleFromThreadTitle) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Санкт-Петербург"))
      .WillOnce(Return(std::string("c2")));
  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));

  nlohmann::json response = {{"segments",
                              {{{"thread", {{"title", "Ласточка"}}},
                                {"departure", "2026-03-25T10:00:00+03:00"},
                                {"arrival", "2026-03-25T14:30:00+03:00"},
                                {"has_transfers", false}}}}};
  EXPECT_CALL(api, Search("c2", "c213", "2026-03-25"))
      .WillOnce(Return(response));

  Router router(api, stations);
  auto result = router.FindRoutes("Санкт-Петербург", "Москва", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ((*result)[0].segments[0].carrier, "Ласточка");
}

TEST(Router, CarrierTitleEmpty) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Санкт-Петербург"))
      .WillOnce(Return(std::string("c2")));
  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));

  nlohmann::json response = {{"segments",
                              {{{"thread", nlohmann::json::object()},
                                {"departure", "2026-03-25T10:00:00+03:00"},
                                {"arrival", "2026-03-25T14:30:00+03:00"},
                                {"has_transfers", false}}}}};
  EXPECT_CALL(api, Search("c2", "c213", "2026-03-25"))
      .WillOnce(Return(response));

  Router router(api, stations);
  auto result = router.FindRoutes("Санкт-Петербург", "Москва", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ((*result)[0].segments[0].carrier, "");
}

TEST(Router, ParsesTransferRoute) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));
  EXPECT_CALL(stations, FindCode("Казань"))
      .WillOnce(Return(std::string("c43")));

  nlohmann::json response = {
      {"segments",
       {{{"departure", "2026-03-25T08:00:00+03:00"},
         {"arrival", "2026-03-25T18:00:00+03:00"},
         {"has_transfers", true},
         {"details",
          nlohmann::json::array(
              {{{"departure", "2026-03-25T08:00:00+03:00"},
                {"arrival", "2026-03-25T12:30:00+03:00"},
                {"thread", {{"carrier", {{"title", "РЖД"}}}}}},
               {{"is_transfer", true},
                {"transfer_point", {{"title", "Иваново"}}}},
               {{"departure", "2026-03-25T13:00:00+03:00"},
                {"arrival", "2026-03-25T18:00:00+03:00"},
                {"thread", {{"carrier", {{"title", "ФПК"}}}}}}})}}}}};
  EXPECT_CALL(api, Search("c213", "c43", "2026-03-25"))
      .WillOnce(Return(response));

  Router router(api, stations);
  auto result = router.FindRoutes("Москва", "Казань", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 1);
  ASSERT_EQ((*result)[0].segments.size(), 2);
  EXPECT_EQ((*result)[0].segments[0].carrier, "РЖД");
  EXPECT_EQ((*result)[0].segments[0].departure_at, "08:00");
  EXPECT_EQ((*result)[0].segments[0].arrives_at, "12:30");
  EXPECT_EQ((*result)[0].segments[1].carrier, "ФПК");
  EXPECT_EQ((*result)[0].segments[1].departure_at, "13:00");
  EXPECT_EQ((*result)[0].segments[1].arrives_at, "18:00");
  ASSERT_EQ((*result)[0].transfer_stations.size(), 1);
  EXPECT_EQ((*result)[0].transfer_stations[0], "Иваново");
}

TEST(Router, TransferDetailMissingThread) {
  MockApiClient api;
  MockStationRepository stations;

  EXPECT_CALL(stations, FindCode("Москва"))
      .WillOnce(Return(std::string("c213")));
  EXPECT_CALL(stations, FindCode("Казань"))
      .WillOnce(Return(std::string("c43")));

  nlohmann::json response = {
      {"segments",
       {{{"departure", "2026-03-25T08:00:00+03:00"},
         {"arrival", "2026-03-25T18:00:00+03:00"},
         {"has_transfers", true},
         {"details", nlohmann::json::array(
                         {{{"departure", "2026-03-25T08:00:00+03:00"},
                           {"arrival", "2026-03-25T12:30:00+03:00"},
                           {"thread", {{"carrier", {{"title", "РЖД"}}}}}},
                          {{"is_transfer", true},
                           {"transfer_point", {{"title", "Иваново"}}}},
                          {{"departure", "2026-03-25T13:00:00+03:00"},
                           {"arrival", "2026-03-25T18:00:00+03:00"}}})}}}}};
  EXPECT_CALL(api, Search("c213", "c43", "2026-03-25"))
      .WillOnce(Return(response));

  Router router(api, stations);
  auto result = router.FindRoutes("Москва", "Казань", "2026-03-25");
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ((*result)[0].segments.size(), 1);
}
