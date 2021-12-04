/*! @file
 * @author Ondrejó András
 * @date 2021.12.02
 */
#pragma once
#include <string>
#include <map>
#include <functional>
#include <span>
#include <string_view>
#include <memory>
#include <vector>
#include "../http/handle.hpp"

namespace tepertokrem {
class VariableRouter;

class Router {
 public:
  virtual ~Router() = default;

  struct Methods {
    http::HandleFunc func;
    std::vector<std::string> methods;
  };

  struct Variable {
    std::string name;
  };

  Router &operator/(std::string_view subroute);
  Router &operator+(Methods m);
  VariableRouter &operator/(Variable variable);

  http::HandleFunc FindHandleFunc(http::Request *request);
  virtual http::HandleFunc FindHandleFuncRecursive(http::Request *request, std::string_view it, std::span<std::string_view> parts);
 private:
  std::map<std::string_view, Router> routes_;
  std::map<std::string, http::HandleFunc> handles_;
  std::unique_ptr<VariableRouter> variable_;
};

class VariableRouter: public Router {
 public:
  explicit VariableRouter(std::string varname);
  http::HandleFunc FindHandleFuncRecursive(http::Request *request, std::string_view it, std::span<std::string_view> parts) override;
 private:
  std::string varname_;
};
}