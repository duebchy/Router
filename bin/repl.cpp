#include "repl.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include "router/router.hpp"

static std::string NormalizeDate(const std::string& s) {
  if (s.size() < 8 || s[2] != '.' || s[5] != '.') return s;
  auto day = s.substr(0, 2);
  auto month = s.substr(3, 2);
  auto year = s.substr(6);
  if (year.size() == 2) year = "20" + year;
  if (year.size() != 4) return s;
  return year + "-" + month + "-" + day;
}

static void PrintHistory(Cache& cache) {
  auto keys = cache.Keys();
  if (keys.empty()) {
    std::cout << "  history is empty\n";
    return;
  }
  for (auto& key : keys) {
    auto p1 = key.find('|');
    auto p2 = key.find('|', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) continue;
    std::cout << "  " << key.substr(0, p1) << " -> "
              << key.substr(p1 + 1, p2 - p1 - 1) << "  " << key.substr(p2 + 1)
              << "\n";
  }
}

void Run(RouterInterface& router, Cache& cache) {
  std::cout << "Usage: <from> -> <to> <date>  (or 'history' / 'exit')\n";

  std::string line;
  while (std::cout << "> " && std::getline(std::cin, line)) {
    if (line == "exit") break;
    if (line.empty()) continue;

    if (line == "history") {
      PrintHistory(cache);
      continue;
    }

    auto arrow = line.find(" -> ");
    if (arrow == std::string::npos) {
      std::cerr << "error: expected <from> -> <to> <date>\n";
      continue;
    }
    std::string from = line.substr(0, arrow);
    std::string rest = line.substr(arrow + 4);
    auto last_space = rest.rfind(' ');
    if (last_space == std::string::npos) {
      std::cerr << "error: expected <from> -> <to> <date>\n";
      continue;
    }
    std::string to = rest.substr(0, last_space);
    std::string date = NormalizeDate(rest.substr(last_space + 1));

    auto result = router.FindRoutes(from, to, date);
    if (!result) {
      std::cerr << "error: " << result.error() << "\n";
      continue;
    }
    if (result->empty()) {
      std::cout << "  no routes found\n";
      continue;
    }
    std::cout << "  " << result->size() << " route(s):\n";
    std::cout << "  ┌─────────────────────────────────────────\n";
    for (std::size_t n = 0; n < result->size(); ++n) {
      auto& route = (*result)[n];
      auto& first = route.segments[0];
      std::cout << "  │ " << (n + 1) << ".  " << first.carrier << "  "
                << first.departure_at << " → " << first.arrives_at << "\n";
      for (std::size_t i = 1; i < route.segments.size(); ++i) {
        auto& seg = route.segments[i];
        if (i - 1 < route.transfer_stations.size() &&
            !route.transfer_stations[i - 1].empty()) {
          std::cout << "  │     ⇌  пересадка: "
                    << route.transfer_stations[i - 1] << "\n";
        }
        std::cout << "  │     └─ " << seg.carrier << "  " << seg.departure_at
                  << " → " << seg.arrives_at << "\n";
      }
    }
    std::cout << "  └─────────────────────────────────────────\n";
  }
}
