/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#include "stream.hpp"

namespace tepertokrem {
Stream::Stream(int fd): fd_{fd} {}

int Stream::GetFileDescriptor() const {
  return fd_;
}

bool Stream::ShouldRead() const {
  return false;
}

bool Stream::ShouldWrite() const {
  return false;
}

void Stream::Read() {}

void Stream::Write() {}
}
