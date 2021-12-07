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

/**
 * Az url-ben egy darabjat reprezentalja (ami ket / kozott van)
 */
class Router {
 public:
  virtual ~Router() = default;

  /**
   * Leirja, hogy az adott utvonal milyen tipusu (GET, POST ...) kereseket
   * tud kiszolgalni egy fuggvenyel
   */
  struct Methods {
    http::HandleFunc func;
    std::vector<std::string> methods;
  };

  /**
   * Az utvonal egy olyan darabja ahol barmi lehet, valtozokent lesz kezelve
   */
  struct Variable {
    std::string name;
  };

  /**
   * Utvonalak osszefuzese
   */
  Router &operator/(std::string_view subroute);
  VariableRouter &operator/(Variable variable);

  Router &operator+(Methods m);

  /**
   * Megkeresi a fugvenyt ami egy kerest ki tud szolgalni
   */
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