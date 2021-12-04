#pragma once
#include <filesystem>
#include <functional>
#include <deque>
#include <pthread.h>
#include <unistd.h>
#include "../stream/stream2.hpp"
#include <magic.h>

namespace tepertokrem {
class FileLoader {
 public:
  explicit FileLoader(int event);
  ~FileLoader();

  struct Task {
    std::filesystem::path file;
    std::function<void(std::vector<char>&&, std::string_view mime)> on_success;
    std::function<void()> on_failure;
  };
  void AddTask(Task task);
 private:
  int event_;
  magic_t magic_;

  void Loop();
  void Process(Task &task);

  std::deque<Task> tasks_;
  pthread_t thread_;
  bool should_exit_ = false;
  pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t has_task_ = PTHREAD_COND_INITIALIZER;
};
}