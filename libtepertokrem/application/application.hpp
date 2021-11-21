/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#pragma once

#include <vector>
#include <memory>
#include <set>
#include "../address.hpp"
#include "../utility/named_type.hpp"
#include "../types.hpp"
#include "../stream/stream.hpp"

namespace tepertokrem {
class Application {
 public:
  ~Application();

  static Application &Instance();

  int ListenAndServe(Address address);
  void Stop();

  void RemoveStreamLater(Stream *s);
 private:
  Application();

  using EPollInstance = NamedType<int, struct EPollInstanceTag>;
  EPollInstance epoll_fd_;

  ServerSocket ssock_;

  int EventLoop();

  void AddToEPoll(Stream *s);
  void UpdateInEPoll(Stream *s);

  std::vector<std::unique_ptr<Stream>> streams_;
  std::set<Stream*> remove_list_;
  void RemoveStreams();

};
}