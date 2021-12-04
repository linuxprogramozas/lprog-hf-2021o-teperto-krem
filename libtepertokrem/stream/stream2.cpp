/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#include <iostream>
#include "stream2.hpp"
#include "../application/application.hpp"

namespace tepertokrem {
Stream2::Stream2(coro_handle handle): handle_{handle} {}

Stream2::~Stream2() {
  if (handle_) {
    handle_.promise().should_close = true;
    if (!handle_.done())
      handle_.resume();
    if (handle_)
      handle_.destroy();
  }
}

ClientSocket Stream2::GetFileDescriptor() const {
  return handle_.promise().csock;
}

void Stream2::Read() {
  if (handle_) {
    handle_.promise().can_read = true;
    if (!handle_.done())
      handle_.resume();
  }
}

void Stream2::Write() {
  if (handle_) {
    handle_.promise().can_write = true;
    if (!handle_.done())
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

void Stream2::AddToWriteBuffer(const std::vector<char> &buf) {
  if (handle_) {
    handle_.promise().write_buffer.insert(handle_.promise().write_buffer.end(), buf.begin(), buf.end());
    if (!handle_.done())
      handle_.resume();
  }
}

void Stream2::AddToWriteBuffer(const std::stringstream &buf) {
  if (handle_) {
    handle_.promise().write_buffer.insert(handle_.promise().write_buffer.end(),
                                          buf.rdbuf()->view().begin(), buf.rdbuf()->view().end());
    if (!handle_.done()) {
      handle_.resume();
    }
  }
}

void Stream2::SetSelf(StreamContainer *container) {
  if (handle_) {
    handle_.promise().self = container;
    if (!handle_.done()) {
      handle_.resume();
    }
  }
}

Stream2::operator bool() const {
  if (handle_) {
    return !handle_.done();
  }
  return false;
}

std::suspend_always Stream2::promise_type::yield_value(ClientSocket csock) {
  this->csock = csock;
  return std::suspend_always{};
}

std::suspend_never Stream2::promise_type::yield_value(http::Request *request) {
  can_write = false;
  can_read = false;
  Application::Instance().ProcessHttp(self, request);
  return std::suspend_never{};
}

bool Stream2::StreamEnableRW::await_ready() const noexcept {
  return false;
}

void Stream2::StreamEnableRW::await_suspend(coro_handle handle) noexcept {
  promise = &handle.promise();
  const bool kChangeHappened =
      (read.value != handle.promise().need_read) || (write.value != handle.promise().need_write);
  handle.promise().need_read = read.value;
  handle.promise().need_write = write.value;
  handle.promise().can_read = false;
  handle.promise().can_write = false;
  if (kChangeHappened) {
    // Modosit
    Application::Instance().UpdateInEPoll(handle.promise().self);
  }
}

Stream2::StreamEvent Stream2::StreamEnableRW::await_resume() const noexcept {
  StreamEvent event;
  if (promise != nullptr) {
    event.read = promise->can_read;
    event.write = promise->can_write;
    event.close = promise->should_close;
    event.write_buffer = &promise->write_buffer;
  }
  return event;
}

}