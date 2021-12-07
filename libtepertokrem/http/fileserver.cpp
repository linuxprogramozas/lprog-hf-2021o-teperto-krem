#include "fileserver.hpp"

namespace tepertokrem::http {

/**
 * Anonim nevter, hogy ne utkozzon veletlenul massal
 */
namespace {
/**
 * Tenylegesen ez felel a fajl kiszolgalasert
 * Sajnos nem lehet lambda mert a lambda capturet ki kell menteni az elso
 * suspension point elott, ez viszont lehetetlen, mert az initial_suspend az std::suspend_always
 */
Handle ServeFile(ResponseWriter w, http::Request *r, std::filesystem::path base_dir) {
  auto dir = base_dir;
  for (const auto& p: r->parsed_url_) {
    dir /= p;
  }
  if (r->parsed_url_.empty()) dir /= "index.html";
  auto[file, mime] = co_await Handle::LoadFile{dir};
  if (file) {
    w.Body().write(file->data(), file->size());
  } else {
    co_return Status::NOT_FOUND;
  }
  auto mime_str = std::string(mime);
  if (mime.starts_with("text"))
    mime_str += "; charset=utf-8";
  w.Header()["Content-Type"].emplace_back(mime_str);
  co_return Status::OK;
}
}

std::function<Handle(ResponseWriter, Request *)> FileServer(std::filesystem::path base_dir) {
  /**
   * A ServeFile lambdaba csomagolva, igy mar szabalyos mert a ServeFile meghivasakor
   * a parameterei a coroutine state be masolodnak
   */
  return [base_dir](const ResponseWriter& w, Request *r) -> Handle {
    return ServeFile(w, r, base_dir);
  };
}

}