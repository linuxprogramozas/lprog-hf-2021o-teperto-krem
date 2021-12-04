/*! @file
 * @author Ondrejó András
 * @date 2021.12.02
 */
#include "handle.hpp"
#include <sstream>
#include <utility>
#include "../stream/stream2.hpp"
#include "../application/application.hpp"

namespace tepertokrem::http {
Handle::Handle(coro_handle handle): handle_{handle} {}

Handle::Handle(Handle &&other) noexcept {
  handle_ = other.handle_;
  other.handle_ = coro_handle{};
}

Handle &Handle::operator=(Handle &&other) noexcept {
  if (this == &other)
    return *this;
  if (handle_) {
    handle_.destroy();
  }
  handle_ = other.handle_;
  other.handle_ = coro_handle {};
  return *this;
}

Handle::~Handle() {
  if (handle_) {
    handle_.destroy();
  }
}

Handle::operator bool() const {
  if (handle_) {
    return !handle_.done();
  }
  return false;
}

void Handle::SetResponseWriter(ResponseWriter writer) {
  handle_.promise().writer = writer;
}

std::suspend_always Handle::promise_type::final_suspend() noexcept {
  std::stringstream ss;
  ss << "HTTP/1.1 " << int(status) << "\r\n";
  for (auto &[key, values]: writer.Header()) {
    for (auto &value: values) {
      ss << key << ": " << value << "\r\n";
    }
  }
  ss << "Content-Length: " << writer.Body().view().size() << "\r\n";
  ss << "\r\n";
  ss.write(writer.Body().view().data(), writer.Body().view().size());
  writer.Stream()->stream.AddToWriteBuffer(ss);
  return std::suspend_always{};
}

Handle::LoadFile::LoadFile(std::filesystem::path file): file_{std::move(file)} {
}

bool Handle::LoadFile::await_ready() const noexcept {
  return false;
}

void Handle::LoadFile::await_suspend(coro_handle handle) noexcept {
  handle_ = handle;
  Application::Instance().LoadFileHttp(handle, file_);
}

std::tuple<Handle::FileResultValue, std::string_view>Handle::LoadFile::await_resume() const noexcept {
  return  {std::move(handle_.promise().file_result_value), handle_.promise().file_result_mime};
}

}