/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#include "stream2.hpp"
#include "../application/application.hpp"

namespace tepertokrem {
Stream2::Stream2(coro_handle handle): handle_{handle} {}

Stream2::~Stream2() {
  if (handle_) {
    handle_.promise().should_close = true;
    handle_.resume();
  }
}

void Stream2::Read() {
  if (handle_) {
    handle_.promise().can_read = true;
    handle_.resume();
  }
}

void Stream2::Write() {
  if (handle_) {
    handle_.promise().can_write = false;
    handle_.resume();
  }
}

bool Stream2::NeedRead() const {
  if (handle_) {
    return handle_.promise().need_read;
  }
  return false;
}

bool Stream2::NeedWrite() const {
  if (handle_) {
    return handle_.promise().need_write;
  }
  return false;
}

Stream2::operator bool() const {
  return bool(handle_);
}

std::suspend_never Stream2::promise_type::yield_value(ClientSocket csock) {
  this->csock = csock;
  // Regisztracio
  return std::suspend_never{};
}

constexpr bool Stream2::StreamEnableRW::await_ready() const noexcept {
  return false;
}

void Stream2::StreamEnableRW::await_suspend(coro_handle handle) noexcept {
  this->handle = &handle;
  const bool kChangeHappened =
      (read.value != handle.promise().need_read) || (write.value != handle.promise().need_write);
  handle.promise().need_read = read.value;
  handle.promise().need_write = write.value;
  if (kChangeHappened) {
    // Modosit
  }
}

Stream2::StreamEvent Stream2::StreamEnableRW::await_resume() const noexcept {
  StreamEvent event;
  if (handle != nullptr) {
    event.read = handle->promise().can_read;
    event.read = handle->promise().can_write;
    event.close = handle->promise().should_close;
  }
  return event;
}

}