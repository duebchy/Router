#include <gtest/gtest.h>

#include "api/yandex_client.hpp"
#include "mocks.hpp"

using ::testing::Return;

TEST(YandexApiClient, ReturnsErrorOnNetworkFailure) {
    MockHttpSession session;
    EXPECT_CALL(session, get(::testing::_))
        .WillOnce(Return(HttpResponse{.has_error = true, .error_message = "connection refused"}));

    YandexApiClient client("key", session);
    auto result = client.Search("c2", "c213", "2026-03-25");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "connection refused");
}

TEST(YandexApiClient, Returns401Error) {
    MockHttpSession session;
    EXPECT_CALL(session, get(::testing::_))
        .WillOnce(Return(HttpResponse{.status_code = 401}));

    YandexApiClient client("bad_key", session);
    auto result = client.Search("c2", "c213", "2026-03-25");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "wrong API key");
}

TEST(YandexApiClient, Returns429Error) {
    MockHttpSession session;
    EXPECT_CALL(session, get(::testing::_))
        .WillOnce(Return(HttpResponse{.status_code = 429}));

    YandexApiClient client("key", session);
    auto result = client.Search("c2", "c213", "2026-03-25");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "reached limit of requests");
}

TEST(YandexApiClient, ReturnsUnknownHttpError) {
    MockHttpSession session;
    EXPECT_CALL(session, get(::testing::_))
        .WillOnce(Return(HttpResponse{.status_code = 503}));

    YandexApiClient client("key", session);
    auto result = client.Search("c2", "c213", "2026-03-25");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "HTTP 503");
}

TEST(YandexApiClient, ParsesJsonResponse) {
    MockHttpSession session;
    EXPECT_CALL(session, get(::testing::_))
        .WillOnce(Return(HttpResponse{
            .status_code = 200,
            .text        = R"({"segments":[]})",
        }));

    YandexApiClient client("key", session);
    auto result = client.Search("c2", "c213", "2026-03-25");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->contains("segments"));
}

TEST(YandexApiClient, ReturnsJsonParseError) {
    MockHttpSession session;
    EXPECT_CALL(session, get(::testing::_))
        .WillOnce(Return(HttpResponse{.status_code = 200, .text = "not json {{{"}));

    YandexApiClient client("key", session);
    auto result = client.Search("c2", "c213", "2026-03-25");
    EXPECT_FALSE(result.has_value());
    EXPECT_THAT(result.error(), ::testing::StartsWith("json parse error"));
}
