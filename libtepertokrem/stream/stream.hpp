/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#pragma once

namespace tepertokrem {

class Stream {
 public:
  Stream(int fd);
  virtual ~Stream() = default;

  [[nodiscard]] int GetFileDescriptor() const;

  virtual bool ShouldRead() const;
  virtual bool ShouldWrite() const;

  virtual void Read();
  virtual void Write();

 protected:
  int fd_;
};

}