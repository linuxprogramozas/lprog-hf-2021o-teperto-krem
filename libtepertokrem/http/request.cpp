/*! @file
 * @author Ondrejó András
 * @date 2021.11.29
 */
#include "request.hpp"
#include <iostream>
#include <sstream>


namespace tepertokrem::http {

std::vector<char> &Request::GetInputBuffer() {
  return input_buffer_;
}

bool Request::ParseInput() {
  std::string_view view(input_buffer_.data(), input_buffer_.size());
  if (header_.empty()) {
    auto end_pos = view.find("\r\n\r\n");
    if (end_pos != std::string_view::npos) {
      // Csak a fejresz minden sor \r\n-el (vagy legalabb \n-el) zarva
      std::string_view header_view{view.begin(), view.begin() + end_pos + 2};
      auto request_line_end = header_view.find_first_of('\n');
      std::string_view request_line{header_view.begin(), header_view.begin() + request_line_end};
      {
        std::stringstream ss{std::string{request_line}};
        ss >> method_;
        ss >> url_;
      }
      // Akkor igaz a request_line_end nem end_pos masodik karaktere (\n)
      // Szoval van meg mit olvasni a headerbol
      if (request_line_end < end_pos) {
        auto start = request_line_end + 1;
        do {
          auto line_end = header_view.find_first_of('\n', start);
          std::string_view header_line{header_view.begin() + start, header_view.begin() + line_end};
          start = line_end + 1;
          if (auto colon = header_line.find_first_of(':'); colon != std::string_view::npos) {
            std::string_view key{header_line.begin(), header_line.begin() + colon};
            if (key.length() > 0) {
              key.remove_prefix(key.find_first_not_of(' '));
            }
            std::string_view value{header_line.begin() + colon + 1, header_line.end()};
            if (value.length() > 0) {
              value.remove_prefix(value.find_first_not_of(' '));
              if (*(value.end() - 1) == '\r') value.remove_suffix(1);
              auto trim_pos = value.find_last_not_of(' ');
              value.remove_suffix(value.length() - trim_pos - 1);
            }
            header_[key] = value;
          }
        } while (start < end_pos);
      }
      if (header_.contains("content-length")) {
        std::stringstream ss{std::string{header_["content-length"]}};
        ss >> content_length_;
        body_start_pos_ = end_pos + 4;
      }
    }
    else return false;
  }
  if (content_length_ == 0) {
    return true;
  }
  else if (body_start_pos_ + content_length_ >= input_buffer_.size()) {
    body_ = {input_buffer_.begin() + body_start_pos_,
             input_buffer_.begin() + body_start_pos_ + content_length_};
    return true;
  }
  return false;
}

std::string_view Request::Method() const {
  return method_;
}

std::string_view Request::Url() const {
  return url_;
}

std::string_view Request::GetBody() const {
  return body_;
}

}