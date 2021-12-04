#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <tuple>

namespace tepertokrem::http {
class Header {
 public:
  void Add(std::string_view key, std::string_view value);
  [[nodiscard]] std::vector<std::string_view> Get(std::string_view key) const;
  [[nodiscard]] bool Empty() const;
 private:
  std::vector<std::tuple<std::string_view, std::vector<std::string_view>>> values_;
};
}