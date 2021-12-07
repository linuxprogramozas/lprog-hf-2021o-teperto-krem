/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#pragma once
#include <concepts>

namespace tepertokrem {
/**
 * Eros tipusossaghoz, hogy a client es server socket int-jet ne lehessen veletlen osszekeverni
 */
template<std::default_initializable UNDERLYING_TYPE, typename TAG>
struct NamedType {
  using UnderlyingType = UNDERLYING_TYPE;
  using Tag = TAG;

  UNDERLYING_TYPE value = UNDERLYING_TYPE{};

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