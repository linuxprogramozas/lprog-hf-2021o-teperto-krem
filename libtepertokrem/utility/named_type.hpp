/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#pragma once
#include <concepts>

namespace tepertokrem {
template<std::default_initializable UNDERLYING_TYPE, typename TAG>
struct NamedType {
  using UnderlyingType = UNDERLYING_TYPE;
  using Tag = TAG;

  UNDERLYING_TYPE value;

  explicit operator UNDERLYING_TYPE&() {
    return value;
  }
  explicit operator const UNDERLYING_TYPE&() const {
    return value;
  }

  NamedType() = default;

  explicit NamedType(const UNDERLYING_TYPE &value): value{value} {}

  NamedType(const NamedType &other): value{other.value} {}

  NamedType &operator=(const UNDERLYING_TYPE &value) {
    this->value = value;
    return *this;
  }

  NamedType &operator=(const NamedType &other) {
    value = other.value;
    return *this;
  }

};
}