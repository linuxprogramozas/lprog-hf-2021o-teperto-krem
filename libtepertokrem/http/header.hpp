#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <tuple>

namespace tepertokrem::http {
/**
 * Fejlec csak kereshez van hasznalva
 */
class Header {
 public:
  void Add(std::string_view key, std::string_view value);

  /**
   * Kulcs alapjan (case insensitive) leker egy ertek hlamazt a fejlecbol
   */
  [[nodiscard]] std::vector<std::string_view> Get(std::string_view key) const;

  [[nodiscard]] bool Empty() const;
 private:
  std::vector<std::tuple<std::string_view, std::vector<std::string_view>>> values_;
};
}