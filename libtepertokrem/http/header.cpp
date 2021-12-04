#include "header.hpp"
#include "strings.h"

namespace tepertokrem::http {
namespace {
struct StrICmp {
  inline bool operator()(const std::string_view &lhs, const std::string_view &rhs) const {
    if (lhs.length() != rhs.length())
      return false;
    return strncasecmp(lhs.data(), rhs.data(), lhs.length()) == 0;
  }
};
}

void Header::Add(std::string_view key, std::string_view value) {
  for (auto &[key_local, values]: values_) {
    if (StrICmp{}(key_local, key)) {
      values.emplace_back(value);
      return;
    }
  }
  values_.emplace_back(std::tuple<std::string_view, std::vector<std::string_view>>(key, {value}));
}

std::vector<std::string_view> Header::Get(std::string_view key) const {
  for (const auto&[kKey, kValues]: values_) {
    if (StrICmp{}(kKey, key))
      return kValues;
  }
  return {};
}

bool Header::Empty() const {
  return values_.empty();
}
}
