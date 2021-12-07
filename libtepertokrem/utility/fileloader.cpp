#include "fileloader.hpp"
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <array>
#include <cstdint>

namespace tepertokrem {
FileLoader::FileLoader(EventFileDescriptor event): event_{event} {
  if (pthread_create(
      &thread_,
      nullptr,
      [](void *thread_ptr) -> void* {
        reinterpret_cast<FileLoader*>(thread_ptr)->Loop();
        return nullptr;
      },
      this)) {
  }
}

FileLoader::~FileLoader() {
  pthread_mutex_lock(&mutex_);
  should_exit_ = true;
  pthread_cond_signal(&has_task_);
  pthread_mutex_unlock(&mutex_);
  pthread_join(thread_, nullptr);
}

void FileLoader::Loop() {
  magic_ = magic_open(MAGIC_MIME_TYPE);
  magic_load(magic_, nullptr);
  while (true) {
    pthread_mutex_lock(&mutex_);
    while (tasks_.empty() && !should_exit_) pthread_cond_wait(&has_task_, &mutex_);
    if (should_exit_) {
      pthread_mutex_unlock(&mutex_);
      break;
    }
    auto tasks = std::exchange(tasks_, {});
    pthread_mutex_unlock(&mutex_);
    while (!tasks.empty()) {
      Process(tasks.front());
      tasks.pop_front();
    }
    uint64_t e = 1;
    write(event_.value, &e, sizeof(uint64_t));
  }
  magic_close(magic_);
}

void FileLoader::AddTask(Task task) {
  pthread_mutex_lock(&mutex_);
  tasks_.emplace_back(task);
  pthread_cond_signal(&has_task_);
  pthread_mutex_unlock(&mutex_);
}

void FileLoader::Process(Task &task) {
  if (int fd = open(task.file.c_str(), O_RDONLY); fd >= 0) {
    auto mime = magic_file(magic_, task.file.c_str());
    ssize_t len = 0;
    std::vector<char> data;
    std::array<char, 1024> buf = {};
    do {
      if (len = read(fd, buf.data(), buf.size()); len > 0) {
        data.insert(data.end(), buf.begin(), buf.begin() + len);
      }
    } while (len > 0);
    close(fd);
    if (len == 0) {
      std::string mime_view = mime;
      if (task.file.extension() == ".css") {
        mime_view = "text/css";
      }
      if (task.file.extension() == "/js") {
        mime_view = "application/javascript";
      }
      task.on_success(std::move(data), mime_view);
    }
  }
  else {
    task.on_failure();
  }
}
}