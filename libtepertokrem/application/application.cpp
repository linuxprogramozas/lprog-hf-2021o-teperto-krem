/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#include "application.hpp"
#include <iostream>
#include <stdexcept>
#include <csignal>
#include <sys/epoll.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include "../address.hpp"
#include "../utility/named_type.hpp"
#include "../types.hpp"
#include "../stream/tcpstream.hpp"
#include "../http/http.hpp"

namespace {
volatile std::sig_atomic_t running = 0;
void SignalHandler(int signal) {
  running = 0;
}
}

namespace tepertokrem {
Application &Application::Instance() {
  static Application app;
  return app;
}

void Application::Stop() {
  running = 0;
  streams_.clear();
  if (ssock_.value > 0) {
    shutdown(ssock_.value, SHUT_RDWR);
    close(ssock_.value);
  }
  // Free stuff
}

Application::Application() {
  std::signal(SIGTERM, SignalHandler);
  if (epoll_fd_ = epoll_create(1); epoll_fd_.value < 0) {
    throw std::runtime_error{"epoll_create(1)"};
  }
}

Application::~Application() {
  Stop();
}

int Application::ListenAndServe(Address address) {
  if (ssock_ = ServerSocket{socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)}; ssock_.value < 0) {
    std::cerr << "Socket(PF_INET): " << std::strerror(errno) << std::endl;
    return 1;
  }
  int optval = 1;
  setsockopt(ssock_.value, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  sockaddr_in addr{};
  std::memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = address.AddressU32();
  addr.sin_port = address.PortU16();
  if (auto res = bind(ssock_.value, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)); res < 0) {
    std::cerr << "bind(): " << std::strerror(errno) << std::endl;
    return 1;
  }
  if (listen(ssock_.value, 16)) {
    std::cerr << "listen(): " << std::strerror(errno) << std::endl;
    return 1;
  }
  epoll_event event{};
  std::memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLPRI;
  event.data.ptr = &ssock_;
  if (auto res = epoll_ctl(epoll_fd_.value, EPOLL_CTL_ADD, ssock_.value, &event); res < 0) {
    std::cerr << "epoll_ctl(EPOLL_CTL_ADD): " << std::strerror(errno) << std::endl;
    return 1;
  }
  running = 1;
  return EventLoop();
}

int Application::EventLoop() {
  epoll_event events[32];
  while (running) {
    if (auto epoll_res = epoll_wait(epoll_fd_.value, events, 32, -1); epoll_res == 0) {
      continue;
    }
    else if (epoll_res == -1) {
      if (auto err = errno; err == EINTR) {
        std::cerr << "epoll_wait() interrupted" << std::endl;
        continue;
      }
      else {
        std::cerr << "epoll_wait(): " << std::strerror(err) << std::endl;
        return 1;
      }
    }
    else {
      for (int i = 0; i < epoll_res; ++i) {
        if (events[i].data.ptr == &ssock_) { // Server socket
          if (events[i].events & EPOLLPRI) {
            std::cerr << "EPOLLPRI" << std::endl;
          }
          if (events[i].events & EPOLLIN) { // Bejovo kapcsolat
            for (auto conn = Http(ssock_); conn; conn = Http(ssock_)) {
              auto container = std::make_unique<StreamContainer>(std::move(conn));
              AddToEPoll(container.get());
              container->stream.SetSelf(container.get());
              streams_.emplace_back(std::move(container));
            }
          }
        }
        else {
          if (events[i].events & EPOLLERR) {
            RemoveStreamLater(reinterpret_cast<StreamContainer*>(events[i].data.ptr));
          }
          if (events[i].events & EPOLLRDHUP) {
            RemoveStreamLater(reinterpret_cast<StreamContainer*>(events[i].data.ptr));
          }
          if (events[i].events & EPOLLIN) {
            reinterpret_cast<StreamContainer*>(events[i].data.ptr)->stream.Read();
          }
          if (events[i].events & EPOLLOUT) {
            reinterpret_cast<StreamContainer*>(events[i].data.ptr)->stream.Write();
          }
        }
      }
    }
    RemoveStreams();
  }
  return 0;
}

void Application::AddToEPoll(StreamContainer *s) {
  epoll_event event;
  event.data.ptr = s;
  event.events =
      (s->stream.NeedRead() ? EPOLLIN: 0) | (s->stream.NeedWrite() ? EPOLLOUT : 0) | EPOLLET | EPOLLRDHUP;
  if (auto res = epoll_ctl(epoll_fd_.value, EPOLL_CTL_ADD, s->stream.GetFileDescriptor().value, &event); res < 0) {
    std::cerr << "epoll_ctl(EPOLL_CTL_ADD): " << std::strerror(errno) << std::endl;
  }
}

void Application::UpdateInEPoll(StreamContainer *s) {
  epoll_event event;
  event.data.ptr = s;
  event.events =
      (s->stream.NeedRead() ? EPOLLIN: 0) | (s->stream.NeedWrite() ? EPOLLOUT : 0) | EPOLLET | EPOLLRDHUP;
  if (auto res = epoll_ctl(epoll_fd_.value, EPOLL_CTL_MOD, s->stream.GetFileDescriptor().value, &event); res < 0) {
    std::cerr << "epoll_ctl(EPOLL_CTL_ADD): " << std::strerror(errno) << std::endl;
  }
}

void Application::RemoveStreamLater(StreamContainer *s) {
  remove_list_.insert(s);
}

void Application::RemoveStreams() {
  // Broadcast stuff
  /*
  for (auto &s: remove_list_) {
    auto tcp = dynamic_cast<TcpStream*>(s);
    if (tcp != nullptr) {
      Address address = tcp->GetAddress();
      std::cerr << "Closed " << address.AddressString() << ":" << address.PortString() << std::endl;
    }
  }
   */
  streams_.erase(
      std::remove_if(streams_.begin(), streams_.end(),
                     [this](auto &ptr) { return remove_list_.contains(ptr.get()); }),
                     streams_.end());
  remove_list_.clear();
}

}
