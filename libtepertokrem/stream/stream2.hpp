/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#pragma once
#include <coroutine>
#include <memory>
#include <utility>
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
  Stream2 &operator=(const Stream2&) = delete;
  Stream2 &operator=(Stream2&&) = default;
  ~Stream2();

  ClientSocket GetFileDescriptor() const;

  void Read();
  void Write();

  [[nodiscard]] bool NeedRead() const;
  [[nodiscard]] bool NeedWrite() const;

  void SetSelf(struct StreamContainer *container);

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

    bool await_ready() const noexcept;
    void await_suspend(coro_handle handle) noexcept;
    StreamEvent await_resume() const noexcept;

    promise_type *promise = nullptr; // Privat kene legyen de ugy nem mukodik
  };

 private:
  coro_handle handle_;
};

struct StreamContainer {
  Stream2 stream;
  inline StreamContainer(Stream2 &&stream): stream{std::move(stream)} {}
  StreamContainer(const StreamContainer&) = delete;
  StreamContainer(StreamContainer&&) = default;
  StreamContainer &operator=(const StreamContainer&) = delete;
  StreamContainer &operator=(StreamContainer&&) = default;
};
using StreamPtr = std::unique_ptr<StreamContainer>;

struct Stream2::promise_type {
  bool can_read = false;
  bool can_write = false;
  bool need_read = false;
  bool need_write = false;
  bool should_close = false;

  ClientSocket csock = ClientSocket{-1};

  StreamContainer *self = nullptr;

  using coro_handle = Stream2::coro_handle;

  inline auto get_return_object() { return coro_handle::from_promise(*this); }
  inline auto initial_suspend() { return std::suspend_never{}; }
  inline auto final_suspend() noexcept { return std::suspend_always{}; }
  void return_void() {}
  void unhandled_exception() {}

  std::suspend_always yield_value(ClientSocket csock);

};


}