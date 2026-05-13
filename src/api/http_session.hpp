#pragma once

#include <cpr/cpr.h>
#include <string>
#include <string_view>
#include <unordered_map>

struct HttpResponse {
  bool has_error = false;
  std::string error_message;
  int status_code = 0;
  std::string text;
};

class HttpSessionInterface {
 public:
  virtual ~HttpSessionInterface() = default;

  virtual HttpResponse get(
      const std::unordered_map<std::string, std::string>& params) = 0;
};

class CprHttpSession final : public HttpSessionInterface {
  cpr::Session session_;

 public:
  explicit CprHttpSession(std::string_view url) {
    session_.SetUrl(cpr::Url{std::string(url)});
  }

  HttpResponse get(
      const std::unordered_map<std::string, std::string>& params) override {
    cpr::Parameters cpr_params;
    for (auto& [k, v] : params) cpr_params.Add({k, v});
    session_.SetParameters(std::move(cpr_params));
    auto r = session_.Get();
    return {.has_error     = bool(r.error),
            .error_message = r.error.message,
            .status_code   = static_cast<int>(r.status_code),
            .text          = r.text};
  }
};
