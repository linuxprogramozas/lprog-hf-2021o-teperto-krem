#include "fileserver.hpp"

namespace tepertokrem::http {

namespace {
Handle ServeFile(ResponseWriter w, http::Request *r, std::filesystem::path base_dir) {
  auto dir = base_dir;
  for (const auto& p: r->parsed_url_) {
    dir /= p;
  }
  if (r->parsed_url_.empty()) dir /= "index.html";
  auto[file, mime] = co_await Handle::LoadFile{dir};
  auto mime_str = std::string(mime);
  if (mime.starts_with("text"))
    mime_str += "; charset=utf-8";
  w.Header()["Content-Type"].emplace_back(mime_str);
  if (file) {
    w.Body().write(file->data(), file->size());
  } else {
    co_return Status::NOT_FOUND;
  }
  co_return Status::OK;
}
}

std::function<Handle(ResponseWriter, Request *)> FileServer(std::filesystem::path base_dir) {
  return [base_dir](const ResponseWriter& w, Request *r) -> Handle {
    return ServeFile(w, r, base_dir);
  };
}

}