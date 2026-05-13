#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "stations/station_repository.hpp"

static std::filesystem::path write_temp(const std::string& name,
                                        const std::string& content) {
    auto p = std::filesystem::temp_directory_path() / name;
    std::ofstream f(p);
    f << content;
    return p;
}

TEST(StationRepository, LoadsFromFile) {
    auto tmp = write_temp("lw7_stations.json",
                          R"({"Москва":"c213","Санкт-Петербург":"c2"})");

    StationRepository repo(tmp, "dummy");
    EXPECT_EQ(repo.FindCode("Москва"), "c213");
    EXPECT_EQ(repo.FindCode("Санкт-Петербург"), "c2");

    std::filesystem::remove(tmp);
}

TEST(StationRepository, FindCodeMiss) {
    auto tmp = write_temp("lw7_stations2.json", R"({"Москва":"c213"})");

    StationRepository repo(tmp, "dummy");
    EXPECT_FALSE(repo.FindCode("Несуществующий").has_value());

    std::filesystem::remove(tmp);
}

TEST(StationRepository, ThrowsOnInvalidJson) {
    auto tmp = write_temp("lw7_stations_bad.json", "not valid json {{{");

    EXPECT_THROW(StationRepository(tmp, "dummy"), std::runtime_error);

    std::filesystem::remove(tmp);
}
