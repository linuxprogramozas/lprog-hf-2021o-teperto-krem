/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#include "tcpstream.hpp"
#include <iostream>
#include <utility>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <array>
#include "../application/application.hpp"

namespace tepertokrem {
TcpStream::TcpStream(ClientSocket csock): Stream(csock.value) {}

std::unique_ptr<TcpStream> TcpStream::Accept(ServerSocket ssock) {
  sockaddr_in address;
  socklen_t address_length = sizeof(address);
  std::memset(&address, 0, sizeof(address));
  if (ClientSocket csock{accept(ssock.value, reinterpret_cast<sockaddr*>(&address), &address_length)}; csock.value > 0) {
    const auto kAddress = Address{address.sin_addr.s_addr, address.sin_port};
    fcntl(csock.value, F_SETFL, fcntl(csock.value, F_GETFL, 0) | O_NONBLOCK);
    auto stream = std::make_unique<TcpStream>(csock);
    stream->address_ = kAddress;
    int optval = 1;
    setsockopt(csock.value, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    return stream;
  }
  else if (auto err = errno; err != EAGAIN) {
    std::cerr << "accept(): " << std::strerror(err) << std::endl;
  }
  return nullptr;
}

TcpStream::~TcpStream() {
  if (fd_ != -1) {
    shutdown(fd_, SHUT_RDWR);
    close(fd_);
  }
}

bool TcpStream::ShouldRead() const {
  return true;
}

void TcpStream::Read() {
  std::array<char, 256> buf = {};
  bool can_read = true;
  while (can_read) {
    if (auto len = recv(fd_, buf.data(), buf.size(), 0); len > 0) {
      write(STDOUT_FILENO, buf.data(), len);
      buffer_.insert(buffer_.end(), buf.begin(), buf.begin() + len);
    }
    else if (len == -1) {
      if (auto err = errno; err != EAGAIN) {
        std::cerr << std::strerror(err) << std::endl;
      }
      can_read = false;
    }
    else if (len == 0) {
      //Application::Instance().RemoveStreamLater(this);
      can_read = false;
    }
  }
}

Address TcpStream::GetAddress() const {
  return address_;
}

}

