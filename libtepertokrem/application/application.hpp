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

  /**
   * Az Application egy singleton, ezzel lehet elerni
   * Varazs static van benne a hivasa szalbiztos
   */
  static Application &Instance();

  /**
   * A megadott cim:port-on elkezd kereseket varni
   */
  int ListenAndServe(Address address);

  void Stop();

  /**
   * A bezart kapcsolatokat kesobb torli ki
   */
  void RemoveStreamLater(StreamContainer *s);

  /**
   * Egy kapcsolat beallitasait frissiti az epollban
   */
  void UpdateInEPoll(StreamContainer *s);

  /**
   * Egy http kerest hozzaad a feldolgozando keresek listajahoz
   */
  void ProcessHttp(StreamContainer *s, http::Request *request);

  /**
   * http::Handle ezen at kezdemenyezhet fajlbetoltest
   */
  void LoadFileHttp(http::Handle::coro_handle handle, std::filesystem::path file);

  /**
   * Ha a fajl betoltes megtortent akkor azt ezzel lehet jelezni, hivasa szalbiztos
   * @param handle kezdemenyezo
   * @param data beolvasott adat
   * @param mime az adat mime type-ja
   */
  void FileLoadSuccessHttp(http::Handle::coro_handle handle, std::vector<char> &&data, std::string mime);

  /**
   * Ha a fajl betoltese sikertelen akkot azt ezzel lehet jelezni
   * @param handle kezdemenyezo
   */
  void FileLoadFailureHttp(http::Handle::coro_handle handle);

  /**
   * / utvonal
   */
  Router &RootRouter();
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

  EventFileDescriptor event_;
  FileLoader *loader_ = nullptr;
};
}