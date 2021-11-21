/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#pragma once
#include <string>
#include <cstdint>

namespace tepertokrem {
class Address {
 public:
  Address() = default;
  Address(std::string socket);
  Address(uint32_t address, uint16_t port);

  std::string AddressString() const;
  std::string PortString() const;

  uint32_t AddressU32() const;
  uint16_t PortU16() const;
 private:
  std::string address_;
  std::string port_;
};

inline Address operator "" _ipv4(const char *str, unsigned long) {
  return Address{str};
}
}