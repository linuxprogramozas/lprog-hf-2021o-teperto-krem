/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#include <iostream>
#include <application/application.hpp>
#include <http/handle.hpp>
#include <http/responsewriter.hpp>
#include <http/request.hpp>
#include <http/fileserver.hpp>

using namespace tepertokrem;
using namespace http;

const auto kEzEgyKecskeHtml = R"(
<html>
<head>
  <title>Kocske</title>
</head>
<body>
  Hello kecske
</body>
</html>
)";

Handle Kecske(ResponseWriter w, Request *r) {
  w.Body() << kEzEgyKecskeHtml;
  co_return Status::OK;
}

Handle Bananok(ResponseWriter w, Request *r) {
  w.Header()["Content-Type"].emplace_back("text/plain");
  w.Body() << "Bananok szama " << r->vars_["banana_count"];
  co_return Status::OK;
}

Handle PostTest(ResponseWriter w, Request *r) {
  w.Header()["Content-Type"].emplace_back("text/plain");
  w.Body() << "Koce";
  std::cerr << r->GetBody() << std::endl;
  co_return Status::OK;
}


int main(int argc, char **argv) {
  auto &app = Application::Instance();

  auto base_dir = std::filesystem::current_path();
  if (base_dir.filename() == "cmake-build-debug")
    base_dir = base_dir.parent_path();
  base_dir /= "web";

  app.RootRouter()
    + Router::Methods{FileServer(base_dir), {"GET"}};

  app.RootRouter() / "kec ske"
    + Router::Methods{Kecske, {"GET"}};

  auto &banana = app.RootRouter() / "banan" / Router::Variable{"banana_count"}
    + Router::Methods{Bananok, {"GET"}};

  banana / "nagyon"
    + Router::Methods{Bananok, {"GET"}};

  return app.ListenAndServe(":8080"_ipv4);
}
