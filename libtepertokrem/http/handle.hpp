/*! @file
 * @author Ondrejó András
 * @date 2021.12.02
 */
#pragma once
#include <functional>
#include <coroutine>
#include <filesystem>
#include <optional>
#include "request.hpp"
#include "responsewriter.hpp"
#include "status.hpp"
#include "../utility/named_type.hpp"

namespace tepertokrem {
class Application;
}
namespace tepertokrem::http {
class Handle {
 public:
  struct promise_type;
  using coro_handle = std::coroutine_handle<promise_type>;

  class LoadFile;
  using FileResultValue = std::optional<std::vector<char>>;

  Handle() = default;
  Handle(coro_handle handle);
  Handle(const Handle&) = delete;
  Handle &operator=(const Handle&) = delete;
  Handle(Handle &&other) noexcept;
  Handle &operator=(Handle &&other) noexcept;
  ~Handle();

  explicit operator bool() const;

  void SetResponseWriter(ResponseWriter writer);

 private:
  coro_handle handle_;
  friend class ::tepertokrem::Application;
};

using HandleFunc = std::function<Handle(ResponseWriter w, Request *request)>;


struct Handle::promise_type {
  using coro_handle = Handle::coro_handle;

  inline auto get_return_object() { return coro_handle::from_promise(*this); }
  inline auto initial_suspend() { return std::suspend_always{}; }
  std::suspend_always final_suspend() noexcept;
  inline void return_value(Status status) { this->status = status; }
  void unhandled_exception() {}

  Status status;
  ResponseWriter writer;

  FileResultValue file_result_value;
  std::string_view file_result_mime;
};

class Handle::LoadFile {
 public:
  explicit LoadFile(std::filesystem::path);
  bool await_ready() const noexcept;
  void await_suspend(coro_handle handle) noexcept;
  std::tuple<FileResultValue, std::string_view> await_resume() const noexcept;
 private:
  std::filesystem::path file_;
  coro_handle handle_;
};

}