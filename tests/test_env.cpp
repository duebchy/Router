#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "env.hpp"

TEST(Env, GetEnvHit) {
  setenv("LW7_TEST_VAR", "hello", 1);
  auto val = GetEnv("LW7_TEST_VAR");
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(*val, "hello");
  unsetenv("LW7_TEST_VAR");
}

TEST(Env, GetEnvMiss) {
  unsetenv("LW7_TEST_VAR_MISSING");
  EXPECT_FALSE(GetEnv("LW7_TEST_VAR_MISSING").has_value());
}

TEST(Env, LoadDotenv) {
  auto tmp = std::filesystem::temp_directory_path() / "lw7_test.env";
  {
    std::ofstream f(tmp);
    f << "# comment\n\nLW7_DOTENV_KEY=dotenv_value\n";
  }
  unsetenv("LW7_DOTENV_KEY");
  LoadDotenv(tmp);
  auto val = GetEnv("LW7_DOTENV_KEY");
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(*val, "dotenv_value");
  std::filesystem::remove(tmp);
  unsetenv("LW7_DOTENV_KEY");
}

TEST(Env, LoadDotenvMissingFile) {
  EXPECT_NO_THROW(LoadDotenv("/nonexistent/path/lw7.env"));
}

TEST(Env, LoadDotenvDoesNotOverwrite) {
  auto tmp = std::filesystem::temp_directory_path() / "lw7_test2.env";
  {
    std::ofstream f(tmp);
    f << "LW7_NO_OVERWRITE=from_file\n";
  }
  setenv("LW7_NO_OVERWRITE", "original", 1);
  LoadDotenv(tmp);
  auto val = GetEnv("LW7_NO_OVERWRITE");
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(*val, "original");
  std::filesystem::remove(tmp);
  unsetenv("LW7_NO_OVERWRITE");
}
