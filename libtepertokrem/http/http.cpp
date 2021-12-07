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
#include <array>
#include <vector>
#include <string_view>
#include "request.hpp"
#include "../address.hpp"
#include "../utility/url.hpp"

namespace tepertokrem {
namespace {

/**
 * Tcp kapcsolat letrehozasaert felelos RAII osztaly
 */
struct HttpTcpConnection {
  /**
   * @param ssock server socket
   */
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
  /**
   * Kapcsolat zarasa es logolas
   */
  ~HttpTcpConnection() {
    if (*this) {
      shutdown(csock.value, SHUT_RDWR);
      close(csock.value);
      std::cerr << "Connection closed: " << address.AddressString() << ":" << address.PortString() << std::endl;
    }
  }

  /**
   * @return kapcsolat letrehozas sikeressege
   */
  explicit operator bool() const {
    return csock.value > 0;
  }

  /**
   * Accept utan letrejott client socket
   */
  ClientSocket csock;

  /**
   * A tuloldal cime es a port amivel csatlakozik
   */
  Address address;
};

/**
 * Socketbol olvasas eredemnye
 */
struct ReadResult {
  enum class State { AGAIN, ERROR, CLOSED } state;
  std::vector<char> data;
};

/**
 * Megprobal minden elerheto adatot kiolvasni a socketbol
 * @param csock client socket
 * @return olvasas eredmenye
 */
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
        std::cerr << "recv(): " << std::strerror(err) << std::endl; // baj van, nem tudom mi
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

  /* Hiba */
  if (!conn)
    co_return;

  /* A letrejott client socketet regisztralni kell epoll-ba */
  co_yield conn.csock;

  /* Http keres adatai */
  http::Request request;

  /* Megerkezett-e a teljes keres */
  bool request_received = false;

  /* Esemeny amiert a coroutine felebredt */
  Stream2::StreamEvent event;

  /* Fociklus addig megy amig valamelyik fel nem zarja a kapcsolatot */
  do {
    /* Jelzes az alkalmazas fele, hogy mirol kerek eretsitest */
    Stream2::StreamEnableRW enable_rw{};

    /* Ha meg nem erkezett meg a teljes keres akkor olvasni kell */
    if (!request_received)
      enable_rw.read = Stream2::StreamEnableRead{true};

    /* Ha a kimeneti bufferben van valami akkor azt ki kell irni */
    if (event.write_buffer != nullptr && !event.write_buffer->empty())
      enable_rw.write = Stream2::StreamEnableWrite{true};

    /* A coroutine varakozo allapotba kerul amig egy esemeny miatt fel nem ebred */
    event = co_await Stream2::StreamEnableRW{enable_rw};

    /* Olvasas */
    if (event.read) {
      /* Kiolvas minen elerheto adatot */
      auto result = ReadAll(conn.csock);
      switch (result.state) {
        /* Kesobb meg johet mnajd adat */
        case ReadResult::State::AGAIN: {
          /* Mindent atmasolok a keresbe */
          request.GetInputBuffer().insert(
              request.GetInputBuffer().end(),
              result.data.begin(),
              result.data.end());
          /* Ellenorzom, hogy atjott-e egy teljes keres */
          if (request_received = request.ParseInput(); request_received) {
            /* Ha post form akkor ertelmezem a query stringet */
            if (const auto &values = request.GetHeader().Get("Content-Type"); !values.empty() && std::string_view(values.front()).starts_with("application/x-www-form-urlencoded")) {
              auto query = std::string_view{request.GetBody()};
              if (request.Method() == "POST") {
                std::stringstream ss{std::string(query)};
                std::string field;
                while (std::getline(ss, field, '&')) {
                  if (auto sep = field.find_first_of('='); sep != std::string::npos) {
                    std::string_view key{field.begin(), field.begin() + sep};
                    std::string_view value{field.begin() + sep + 1, field.end()};
                    request.vars_[DecodeUrl(key)] = DecodeUrl(value);
                  }
                }
              }
            }
            /* A keres feldolgozasa mas dolga, ez a co_yield nem suspendel */
            co_yield &request;
          }
          break;
        }
        /* Valami baj van */
        case ReadResult::State::ERROR: {
          // Semmi
          break;
        }
        /* Kapcsolat lezarult */
        case ReadResult::State::CLOSED: {
          co_return;
        }
      }
    }
    /* van irando adat es lehetosegem van irni */
    if (event.write) {
      auto sent = send(conn.csock.value, event.write_buffer->data(), event.write_buffer->size(), MSG_NOSIGNAL);
      if (sent > 0) {
        event.write_buffer->erase(event.write_buffer->begin(), event.write_buffer->begin() + sent);
      }
      /* Ha minden sikerult elkuldeni */
      if (event.write_buffer->empty()) {
        /* ha a kliens kerte a kapcsolat eletben tartasan akkor csak az elozo keres tartalmat torlom es varom a kovetkezot */
        if (const auto &values = request.GetHeader().Get("Connection"); !values.empty() && values.front() == "keep-alive") {
          request = http::Request{};
          request_received = false;
        }
        /* Kliens nem kerte a kapcsolat eletben tartasat ezert zarom */
        else {
          co_return;
        }
      }
    }
    if (event.close) {
      std::cerr << "Close requested" << std::endl;
      break;
    }
  } while (true);
  co_return;
}
}