/*! @file
 * @author Ondrejó András
 * @date 2021.12.02
 */
#include "router.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string_view>
#include "../http/handle.hpp"
#include "../http/request.hpp"
#include "../http/responsewriter.hpp"
#include "../utility/url.hpp"

namespace tepertokrem {
Router &Router::operator/(std::string_view subroute) {
  return routes_[subroute];
}

Router &Router::operator+(Methods m) {
  for (auto method: m.methods) {
    handles_[method] = m.func;
  }
  return *this;
}

http::HandleFunc Router::FindHandleFunc(http::Request *request) {
  std::vector<std::string_view> parts;
  auto url = request->Url();
  if (url != "/") {
    url.remove_prefix(1);
    auto end = std::string_view::npos;
    do {
      if (end = url.find_first_of('/'); end != std::string_view::npos) {
        parts.emplace_back(url.begin(), url.begin() + end);
        url.remove_prefix(end + 1);
      } else {
        parts.emplace_back(url.begin(), url.end());
      }
    } while (end != std::string_view::npos);
    if (parts.back().empty())
      parts.pop_back();
  }
  if (!parts.empty()) {
    auto qmark = parts.back().find_first_of('?');
    if (qmark != std::string_view::npos) {
      auto query = std::string_view{parts.back().begin() + qmark + 1, parts.back().end()};
      if (request->Method() == "GET") {
        std::stringstream ss{std::string(query)};
        std::string field;
        while (std::getline(ss, field, '&')) {
          if (auto sep = field.find_first_of('='); sep != std::string::npos) {
            std::string_view key{field.begin(), field.begin() + sep};
            std::string_view value{field.begin() + sep + 1, field.end()};
            request->vars_[DecodeUrl(key)] = DecodeUrl(value);
          }
        }
      }
      parts.back().remove_suffix(query.size() + 1);
      if (parts.back().empty()) {
        parts.pop_back();
      }
    }
  }
  request->parsed_url_.reserve(parts.size());
  for (auto part: parts) {
    request->parsed_url_.emplace_back(DecodeUrl(part));
  }
  if (!parts.empty()) {
    auto front = DecodeUrl(parts.front());
    if (routes_.contains(front)) {
      if (auto handle = routes_[parts.front()].FindHandleFuncRecursive(request, parts.front(), std::span{parts.begin() + 1,parts.end()}); handle) {
        return handle;
      }
    }
    if (variable_) {
      if (auto handle = variable_->FindHandleFuncRecursive(request, parts.front(), std::span{parts.begin() + 1, parts.end()}); handle)
        return handle;
    }
  }
  if (handles_.contains(std::string(request->Method()))) {
    return handles_[std::string(request->Method())];
  }
  throw std::runtime_error{"can't find route"};
}

http::HandleFunc Router::FindHandleFuncRecursive(http::Request *request, std::string_view it, std::span<std::string_view> parts) {
  if (!parts.empty()) {
    auto front = DecodeUrl(parts.front());
    if (routes_.contains(front)) {
      if (auto handle = routes_[parts.front()].FindHandleFuncRecursive(request, parts.front(), std::span{parts.begin() + 1, parts.end()}); handle) {
        return handle;
      }
    }
    if (variable_) {
      return variable_->FindHandleFuncRecursive(request, parts.front(), std::span{parts.begin() + 1, parts.end()});
    }
  }
  else {
    if (handles_.contains(std::string(request->Method()))) {
      return handles_[std::string(request->Method())];
    }
  }
  return {};
}

VariableRouter &Router::operator/(Variable variable) {
  variable_ = std::make_unique<VariableRouter>(variable.name);
  return *variable_;
}

VariableRouter::VariableRouter(std::string varname): varname_{varname} {
}

http::HandleFunc VariableRouter::FindHandleFuncRecursive(http::Request *request, std::string_view it, std::span<std::string_view> parts) {
  request->vars_[varname_] = DecodeUrl(it);
  return Router::FindHandleFuncRecursive(request, it, parts);
}

}