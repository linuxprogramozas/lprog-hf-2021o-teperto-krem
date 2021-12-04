#include "url.hpp"
#include <sstream>
#include <string_view>
#include <map>

namespace tepertokrem {
namespace {
std::map<std::string_view, char> encoded_characters = {
    {"%20", ' '},
    {"%21", '!'},
    {"%22", '"'},
    {"%23", '#'},
    {"%24", '$'},
    {"%25", '%'},
    {"%26", '&'},
    {"%27", '\''},
    {"%28", '('},
    {"%29", ')'},
    {"%2A", '*'},
    {"%2B", '+'},
    {"%2C", ','},
    {"%2D", '-'},
    {"%2E", '.'},
    {"%2F", '/'},
    {"%3A", ':'},
    {"%3B", ';'},
    {"%3C", '<'},
    {"%3D", '='},
    {"%3E", '>'},
    {"%3F", '?'},
    {"%40", '@'},
    {"%5B", '['},
    {"%5C", '\\'},
    {"%5D", ']'},
    {"%5E", '^'},
    {"%5F", '_'},
    {"%60", '`'},
    {"%7B", '{'},
    {"%7C", '|'},
    {"%7D", '}'},
    {"%7E", '~'}
};
}
std::string DecodeUrl(std::string_view view) {
  std::stringstream ss;
  std::string_view::size_type p;
  while (true) {
    p = view.find_first_of('%');
    if (p == std::string_view::npos)
      break;
    if (encoded_characters.contains(view.substr(p, 3))) {
      auto c = encoded_characters[view.substr(p, 3)];
      ss << std::string(view.substr(0, p)) << c;
      view.remove_prefix(p + 3);
    }
    else {
      ss << std::string(view.substr(0, p + 1));
      view.remove_prefix(p + 1);
    }
  }
  ss << std::string{view.begin(), view.end()};
  return ss.str();
}
}
