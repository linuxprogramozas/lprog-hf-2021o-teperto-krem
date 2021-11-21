/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#pragma once
#include <coroutine>
#include "../utility/named_type.hpp"
#include "../types.hpp"

namespace tepertokrem {

class Stream2 {
 public:
  struct promise_type;
  using coro_handle = std::coroutine_handle<promise_type>;

  Stream2(coro_handle handle);
  Stream2(const Stream2&) = delete;
  Stream2(Stream2&&) = default;
  ~Stream2();

  void Read();
  void Write();

  [[nodiscard]] bool NeedRead() const;
  [[nodiscard]] bool NeedWrite() const;

  explicit operator bool() const;

  using StreamEnableRead = NamedType<bool, struct StreamEnableReadTag>;
  using StreamEnableWrite = NamedType<bool, struct StreamEnableWriteTag>;
  using StreamShouldClose = NamedType<bool, struct StreamShouldCloseTag>;

  struct StreamEvent {
    StreamEnableRead read;
    StreamEnableWrite write;
    StreamShouldClose close;
  };

  struct StreamEnableRW {
    StreamEnableRead read;
    StreamEnableWrite write;

    constexpr bool await_ready() const noexcept;
    void await_suspend(coro_handle handle) noexcept;
    StreamEvent await_resume() const noexcept;
   private:
    coro_handle *handle = nullptr;
  };

 private:
  coro_handle handle_;
};

struct Stream2::promise_type {
  bool can_read = false;
  bool can_write = false;
  bool need_read = false;
  bool need_write = false;
  bool should_close = false;

  ClientSocket csock = ClientSocket{-1};

  using coro_handle = Stream2::coro_handle;

  inline auto get_return_object() { return coro_handle::from_promise(*this); }
  inline auto initial_suspend() { return std::suspend_never{}; }
  inline auto final_suspend() noexcept { return std::suspend_never{}; }
  void return_void() {}
  void unhandled_exception() {}

  std::suspend_never yield_value(ClientSocket csock);

};
}