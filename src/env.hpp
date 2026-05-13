#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

inline std::optional<std::string_view> GetEnv(const char* name) {
  const char* val = std::getenv(name);
  if (val) return val;
  return std::nullopt;
}

inline void LoadDotenv(const std::filesystem::path& path = ".env") {
  std::ifstream f(path);
  if (!f.is_open()) return;
  std::string line;
  while (std::getline(f, line)) {
    if (line.empty() || line.front() == '#') continue;
    auto eq = line.find('=');
    if (eq == std::string::npos) continue;
    setenv(line.substr(0, eq).c_str(), line.substr(eq + 1).c_str(), 0);
  }
}
