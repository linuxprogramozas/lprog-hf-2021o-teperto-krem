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
#include <sstream>
#include <unistd.h>
#include <sys/eventfd.h>
#include "../address.hpp"
#include "../utility/named_type.hpp"
#include "../types.hpp"
#include "../http/http.hpp"
#include "../utility/url.hpp"
#include "../utility/fileloader.hpp"

namespace {
volatile std::sig_atomic_t running = 0;
void SignalHandler([[maybe_unused]] int signal) {
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
  close(event_.value);
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

  event_ = eventfd(0, EFD_NONBLOCK);
  std::memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN;
  event.data.ptr = &event_;
  if (auto res = epoll_ctl(epoll_fd_.value, EPOLL_CTL_ADD, event_.value, &event); res < 0) {
    std::cerr << "epoll_ctl(EPOLL_CTL_ADD): " << std::strerror(errno) << std::endl;
    return 1;
  }

  running = 1;

  FileLoader fl(event_);
  loader_ = &fl;

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
        else if (events[i].data.ptr == &event_) {
          std::array<char, 8> _ = {};
          read(event_.value, _.data(), _.size());
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
    pthread_mutex_lock(&http_handle_mutex_);
    for (auto &handle: http_handles_ready_) { // Vegig iteralok minden futattasra kesz handle-on
      if (!handle.done()) { // Ha nincs kesz futtatom
        handle.resume();
      }
      if (handle.done()) { // Ha elkeszult akkor beteszem a kesz listaba
        http_handles_done_.insert(handle);
      }
    }
    http_handles_ready_.clear(); // Uritem a futtatasra kesz listat
    pthread_mutex_unlock(&http_handle_mutex_);
    RemoveHttpHandles(); // Elobb a http handlek eltavolitasa mert az lehet, hogy magaval vonja egy stream bezarasat is
    RemoveStreams();
  }
  return 0;
}

void Application::AddToEPoll(StreamContainer *s) {
  epoll_event event = {};
  event.data.ptr = s;
  event.events =
      (s->stream.NeedRead() ? uint32_t(EPOLLIN): 0u) | (s->stream.NeedWrite() ? uint32_t(EPOLLOUT) : 0u) | uint32_t(EPOLLET) | uint32_t(EPOLLRDHUP);
  if (auto res = epoll_ctl(epoll_fd_.value, EPOLL_CTL_ADD, s->stream.GetFileDescriptor().value, &event); res < 0) {
    std::cerr << "epoll_ctl(EPOLL_CTL_ADD): " << std::strerror(errno) << std::endl;
  }
}

void Application::UpdateInEPoll(StreamContainer *s) {
  epoll_event event = {};
  event.data.ptr = s;
  event.events =
      (s->stream.NeedRead() ? uint32_t(EPOLLIN): 0u) | (s->stream.NeedWrite() ? uint32_t(EPOLLOUT) : 0u) | uint32_t(EPOLLET) | uint32_t(EPOLLRDHUP);
  if (auto res = epoll_ctl(epoll_fd_.value, EPOLL_CTL_MOD, s->stream.GetFileDescriptor().value, &event); res < 0) {
    std::cerr << "epoll_ctl(EPOLL_CTL_ADD): " << std::strerror(errno) << std::endl;
  }
}

void Application::RemoveStreamLater(StreamContainer *s) {
  remove_list_.insert(s);
}

void Application::RemoveStreams() {
  streams_.erase(
      std::remove_if(streams_.begin(), streams_.end(),
                     [this](auto &ptr) { return remove_list_.contains(ptr.get()); }),
                     streams_.end());
  // Stop http handles
  remove_list_.clear();
}

Router &Application::RootRouter() {
  return root_router_;
}

void Application::ProcessHttp(StreamContainer *s, http::Request *request) {
  try {
    auto func = root_router_.FindHandleFunc(request); // Ez dobhat kivetelt
    auto writer = http::ResponseWriter{s};
    pthread_mutex_lock(&http_handle_mutex_);
    auto &handle = http_handles_[s] = func(writer, request);
    handle.SetResponseWriter(writer);
    http_handles_ready_.insert(handle.handle_);
    pthread_mutex_unlock(&http_handle_mutex_);
  }
  catch (std::runtime_error &ex) {
    std::cerr << ex.what() << std::endl;
    auto err = "HTTP/1.0 404 NOT FOUND\r\n\r\n";
    std::vector<char> data;
    data.resize(std::strlen(err));
    std::memcpy(data.data(), err, std::strlen(err));
    s->stream.AddToWriteBuffer(data);
  }
}

void Application::RemoveHttpHandles() {
  pthread_mutex_lock(&http_handle_mutex_);
  erase_if(http_handles_, [this](const auto &item) -> bool {
    const auto &[kEy, kHandle] = item;
    return http_handles_done_.contains(kHandle.handle_);
  });
  erase_if(http_handle_waiting_, [this](const auto &item) -> bool {
    return http_handles_done_.contains(item);
  });
  pthread_mutex_unlock(&http_handle_mutex_);
  http_handles_done_.clear();
}

void Application::LoadFileHttp(http::Handle::coro_handle handle, std::filesystem::path file) {
  // Ez egy coroutinebol (kb) hivodik
  // Coroutine feldolgozas kozben a mutex lockolva van itt nem kell ujra
  http_handle_waiting_.insert(handle);
  loader_->AddTask({
    .file = file,
    .on_success = [this, handle](std::vector<char> &&data, std::string mime){
      FileLoadSuccessHttp(handle, std::move(data), mime);
    },
    .on_failure = [this, handle](){
      FileLoadFailureHttp(handle);
    }
  });
}

void Application::FileLoadSuccessHttp(http::Handle::coro_handle handle, std::vector<char> &&data, std::string mime) {
  pthread_mutex_lock(&http_handle_mutex_);
  if (http_handle_waiting_.contains(handle)) {
    handle.promise().file_result_value = std::move(data);
    handle.promise().file_result_mime = mime;
    http_handles_ready_.insert(handle);
    http_handle_waiting_.erase(handle);
  }
  pthread_mutex_unlock(&http_handle_mutex_);
}

void Application::FileLoadFailureHttp(http::Handle::coro_handle handle) {
  pthread_mutex_lock(&http_handle_mutex_);
  if (http_handle_waiting_.contains(handle)) {
    handle.promise().file_result_value->clear();
    handle.promise().file_result_mime = {};
    http_handles_ready_.insert(handle);
    http_handle_waiting_.erase(handle);
  }
  pthread_mutex_unlock(&http_handle_mutex_);
}

}
