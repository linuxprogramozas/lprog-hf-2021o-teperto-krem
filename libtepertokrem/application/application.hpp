/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#pragma once

#include <vector>
#include <memory>
#include <set>
#include <map>
#include <pthread.h>
#include "../address.hpp"
#include "../utility/named_type.hpp"
#include "../types.hpp"
#include "../stream/stream.hpp"
#include "../stream/stream2.hpp"
#include "../utility/fileloader.hpp"
#include "router.hpp"

namespace tepertokrem {

namespace http {
class Request;
}

class Application {
 public:
  ~Application();

  static Application &Instance();

  int ListenAndServe(Address address);
  void Stop();

  void RemoveStreamLater(StreamContainer *s);
  void UpdateInEPoll(StreamContainer *s);

  void ProcessHttp(StreamContainer *s, http::Request *request);
  Router &RootRouter();
  void LoadFileHttp(http::Handle::coro_handle handle, std::filesystem::path file);
  void FileLoadSuccessHttp(http::Handle::coro_handle handle, std::vector<char> &&data, std::string_view mime);
  void FileLoadFailureHttp(http::Handle::coro_handle handle);
 private:
  Application();

  using EPollInstance = NamedType<int, struct EPollInstanceTag>;
  EPollInstance epoll_fd_;

  ServerSocket ssock_;

  int EventLoop();

  void AddToEPoll(StreamContainer *s);

  std::vector<StreamPtr> streams_;
  std::set<StreamContainer*> remove_list_;
  void RemoveStreams();

  Router root_router_;
  std::map<StreamContainer*, http::Handle> http_handles_;
  std::set<http::Handle::coro_handle> http_handles_ready_;
  std::set<http::Handle::coro_handle> http_handle_waiting_;
  std::set<http::Handle::coro_handle> http_handles_done_;
  pthread_mutex_t http_handle_mutex_ = PTHREAD_MUTEX_INITIALIZER;
  void RemoveHttpHandles();

  int event_ = 0;
  FileLoader *loader_ = nullptr;
};
}