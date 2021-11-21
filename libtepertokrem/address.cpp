/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#include "address.hpp"
#include <sstream>
#include <array>
#include <stdexcept>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace tepertokrem {
Address::Address(std::string socket) {
  std::stringstream ss{socket};
  std::getline(ss, address_, ':');
  std::getline(ss, port_, ':');
}

Address::Address(uint32_t address, uint16_t port) {
  const uint16_t kPort = ntohs(port);
  std::array<char, sizeof("xxx.xxx.xxx.xxx")> address_buffer = {'\0'};
  if (auto ip_string = inet_ntop(AF_INET, &address, address_buffer.data(), address_buffer.size()); ip_string == nullptr) {
    throw std::runtime_error{"failed to parse address"};
  }
  else {
    address_ = ip_string;
  }
  std::stringstream ss;
  ss << kPort;
  port_ = ss.str();
}

std::string Address::AddressString() const {
  return address_;
}

std::string Address::PortString() const {
  return port_;
}

uint32_t Address::AddressU32() const {
  if (address_.empty() || address_ == "0.0.0.0")
    return 0u;
  if (auto address = inet_addr(address_.c_str()); address == 0) {
    throw std::runtime_error{"failed to parse address"};
  }
  else {
    return address;
  }
}

uint16_t Address::PortU16() const {
  std::stringstream ss{port_};
  uint16_t port;
  ss >> port;
  return htons(port);
}
}
