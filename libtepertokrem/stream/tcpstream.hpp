/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#pragma once
#include <memory>
#include <vector>
#include "stream.hpp"
#include "../address.hpp"
#include "../types.hpp"

namespace tepertokrem {
class TcpStream final: public Stream {
 public:
  explicit TcpStream(ClientSocket csock);
  ~TcpStream() override;

  [[nodiscard]] bool ShouldRead() const override;
  void Read() override;

  static std::unique_ptr<TcpStream> Accept(ServerSocket ssock);

  [[nodiscard]] Address GetAddress() const;
 private:
  Address address_;
  std::vector<char> buffer_;
};
}