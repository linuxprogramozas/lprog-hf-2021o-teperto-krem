/*! @file
 * @author Ondrejó András
 * @date 2021.11.29
 */
#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <utility>
#include "strings.h"
#include "header.hpp"

namespace tepertokrem::http {
/**
 * Http keres tartalma
 */
class Request {
 public:
  Request() = default;
  Request(const Request&) = delete;
  Request &operator=(const Request&) = delete;
  Request(Request&&) noexcept = default;
  Request &operator=(Request&&) noexcept = default;

  std::vector<char> &GetInputBuffer();
  bool ParseInput();


  [[nodiscard]] std::string_view Method() const;
  [[nodiscard]] std::string_view Url() const;

  [[nodiscard]] std::string_view GetBody() const;
  [[nodiscard]] const Header &GetHeader() const;

  std::map<std::string, std::string> vars_;
  std::vector<std::string> parsed_url_;
 private:
  std::vector<char> input_buffer_;

  std::string method_;
  std::string url_;

  Header header_;

  ssize_t content_length_ = 0;
  ssize_t body_start_pos_ = 0;
  std::string_view body_;
};
}