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
#include <vector>
#include <string_view>
#include "request.hpp"

namespace tepertokrem {
namespace {
struct HttpTcpConnection {
  HttpTcpConnection(ServerSocket ssock) {
    sockaddr_in address_2;
    socklen_t address_length = sizeof(address_2);
    std::memset(&address_2, 0, sizeof(address_2));
    if (csock = accept(ssock.value, reinterpret_cast<sockaddr *>(&address_2), &address_length); csock.value > 0) {
      fcntl(csock.value, F_SETFL, fcntl(csock.value, F_GETFL, 0) | O_NONBLOCK);
      int optval = 1;
      setsockopt(csock.value, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
      address = Address{address_2.sin_addr.s_addr, address_2.sin_port};
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

struct ReadResult {
  enum class State { AGAIN, ERROR, CLOSED } state;
  std::vector<char> data;
};

ReadResult ReadAll(ClientSocket csock) {
  ReadResult result;
  std::array<char, 256> buf{};
  while (true) {
    if (auto len = recv(csock.value, buf.data(), buf.size(), 0); len > 0) {
      result.data.insert(result.data.end(), buf.begin(), buf.begin() + len);
    }
    else if (len == -1) {
      if (auto err = errno; err == EAGAIN) {
        result.state = ReadResult::State::AGAIN;
      }
      else {
        result.state = ReadResult::State::ERROR;
        std::cerr << "recv(): " << std::strerror(err) << std::endl; // Szar van, nem tudom mi
      }
      break;
    }
    else if (len == 0) {
      result.state = ReadResult::State::CLOSED;
      break;
    }
  }
  return result;
}

}

Stream2 Http(ServerSocket ssock) {
  HttpTcpConnection conn{ssock};
  if (!conn)
    co_return; // Hiba
  co_yield conn.csock; // Suspend, regisztracio
  std::vector<char> input_buffer;
  http::Request request;
  bool request_received = false;
  do {
    Stream2::StreamEvent event;
    if (!request_received) {
      event = co_await Stream2::StreamEnableRW{.read = Stream2::StreamEnableRead{true}};
    }
    else {
      event = co_await Stream2::StreamEnableRW{};
    }
    if (event.read) { // Olvasas
      auto result = ReadAll(conn.csock);
      switch (result.state) {
        case ReadResult::State::AGAIN: {
          request.GetInputBuffer().insert(
              request.GetInputBuffer().end(),
              result.data.begin(),
              result.data.end());
          if (request_received = request.ParseInput(); request_received) {
            std::cerr << request.Method() << "\t" << request.Url() << std::endl;
            std::cerr << request.GetBody() << std::endl;
            std::cerr << "======ToltsdmarfelaZHt======" << std::endl;
            // TODO: dispatch to http handle
            // TODO:
          }
          break;
        }
        case ReadResult::State::ERROR: {
          // Semmi
          break;
        }
        case ReadResult::State::CLOSED: {
          co_return;
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