/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#include "http.hpp"
#include <iostream>
#include <cerrno>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include "../address.hpp"
#include <array>

namespace tepertokrem {
namespace {
struct HttpTcpConnection {
  HttpTcpConnection(ServerSocket ssock) {
    sockaddr_in address2;
    socklen_t address_length = sizeof(address2);
    std::memset(&address2, 0, sizeof(address2));
    if (csock = accept(ssock.value, reinterpret_cast<sockaddr *>(&address2), &address_length); csock.value > 0) {
      fcntl(csock.value, F_SETFL, fcntl(csock.value, F_GETFL, 0) | O_NONBLOCK);
      int optval = 1;
      setsockopt(csock.value, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
      address = Address{address2.sin_addr.s_addr, address2.sin_port};
    } else if (auto err = errno; err != EAGAIN) {
      std::cerr << "accept(): " << std::strerror(err) << std::endl;
    }
  }
  ~HttpTcpConnection() {
    if (*this) {
      shutdown(csock.value, SHUT_RDWR);
      close(csock.value);
      std::cerr << "Connection closed: " << address.AddressString() << ":" << address.PortString() << std::endl;
    }
  }

  explicit operator bool() const {
    return csock.value > 0;
  }

  ClientSocket csock;
  Address address;
};
}

Stream2 Http(ServerSocket ssock) {
  HttpTcpConnection conn{ssock};
  if (!conn)
    co_return; // Hiba
  co_yield conn.csock;
  do {
    auto event = co_await Stream2::StreamEnableRW{.read = Stream2::StreamEnableRead{true}};
    if (event.read) { // Olvasas
      std::array<char, 256> buf{};
      bool can_read = true;
      while (can_read) {
        if (auto len = recv(conn.csock.value, buf.data(), buf.size(), 0); len > 0) {
          write(STDOUT_FILENO, buf.data(), len);
        } else if (len == -1) {
          if (auto err = errno; err != EAGAIN) {
            std::cerr << std::strerror(err) << std::endl; // Szar van, nem tudom mi
          }
          can_read = false; // Elfogyott az olvashato adat
        } else if (len == 0) {
          co_return; // Kapcsolat lezarva
        }
      }
    }
    // TODO iras
    if (event.close) {
      std::cerr << "Close requested" << std::endl;
      break;
    }
  } while (true);
  co_return;
}
}