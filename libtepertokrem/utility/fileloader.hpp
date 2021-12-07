#pragma once
#include <filesystem>
#include <functional>
#include <deque>
#include <pthread.h>
#include <unistd.h>
#include <magic.h>
#include "../stream/stream2.hpp"
#include "../types.hpp"

namespace tepertokrem {
/**
 * Fajlbetolto kulon szalon, mert a normalis fajlok nem mukodnek epollal
 */
class FileLoader {
 public:
  /**
   * @param event eventfd amivel a foszalat ebreszteni tudja
   */
  explicit FileLoader(EventFileDescriptor event);
  ~FileLoader();

  /**
   * Elvegzendo feladat
   * Azt, hogy milyen modon kell visszajeleznie nem tudja, csak egy
   * fuggvenyt kap ra amit meg lehet hivni
   */
  struct Task {
    std::filesystem::path file;
    std::function<void(std::vector<char>&&, std::string mime)> on_success;
    std::function<void()> on_failure;
  };
  void AddTask(Task task);
 private:
  EventFileDescriptor event_;

  /**
   * mime type kitalalasahoz
   */
  magic_t magic_;

  /**
   * Ebben varja az uj feladatokat, kulon szalon
   */
  void Loop();

  /**
   * Feldolgoz egy feladatot
   */
  void Process(Task &task);

  /**
   * Feladat varolista
   */
  std::deque<Task> tasks_;
  pthread_t thread_;
  bool should_exit_ = false;
  pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t has_task_ = PTHREAD_COND_INITIALIZER;
};
}