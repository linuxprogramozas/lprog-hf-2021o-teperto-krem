/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#include <iostream>
#include <array>
#include <memory>
#include <sstream>
#include <tuple>
#include <optional>
#include <vector>
#include <random>
#include <algorithm>

#include <application/application.hpp>
#include <http/handle.hpp>
#include <http/responsewriter.hpp>
#include <http/request.hpp>
#include <http/fileserver.hpp>

using namespace tepertokrem;
using namespace http;

struct Board {
  std::array<char, 9> fields;
};

std::optional<std::tuple<int, int>> ChooseRandomOnBoard(Board *board);

std::string CheckBoardState(Board *board);

Handle Place(ResponseWriter w, Request *r, Board *board);

Handle GetBoard(ResponseWriter w, [[maybe_unused]] Request *r, Board *board);

Handle ResetBoard(ResponseWriter w, Request *r, Board *board);

struct BevasarloLista {
  std::vector<std::string> aruk;
};

Handle BevasarloListaAdd(ResponseWriter w, Request *r, BevasarloLista *lista) {
  lista->aruk.emplace_back(r->vars_["aruk"]);
  std::cerr << "Aru hozzaadva " << r->vars_["aruk"] << std::endl;
  w.Header()["location"].emplace_back("/bevasarlolista.html");
  co_return Status::SEE_OTHER;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  auto &app = Application::Instance();

  auto base_dir = std::filesystem::current_path();
  if (base_dir.filename() == "cmake-build-debug" || base_dir.filename() == "cmake-build-release")
    base_dir = base_dir.parent_path();
  base_dir /= "web";

  auto board = std::make_unique<Board>();
  std::fill(board->fields.begin(), board->fields.end(), ' ');

  app.RootRouter()
      + Router::Methods{FileServer(base_dir), {"GET"}};

  auto &board_router = app.RootRouter() / "board"
      + Router::Methods{[&board](ResponseWriter w, Request *r) {
        return GetBoard(w, r, board.get());
      }, {"GET"}};

  board_router / "place"
      + Router::Methods{[&board](ResponseWriter w, Request *r) {
        return Place(w, r, board.get());
      }, {"POST"}};

  board_router / "reset"
      + Router::Methods{[&board](ResponseWriter w, Request *r) {
        return ResetBoard(w, r, board.get());
      }, {"POST"}};

  auto lista = std::make_unique<BevasarloLista>();

  Application::Instance().RootRouter() / "bevasarlolista" / "add"
      + Router::Methods{[&lista](ResponseWriter w, Request *r) {
        return BevasarloListaAdd(w, r, lista.get());
      }, {"GET"}};

  return app.ListenAndServe(":8080"_ipv4);
}

Handle Place(ResponseWriter w, Request *r, Board *board) {
  std::stringstream param_parser;
  param_parser << r->vars_["x"] << ' ' << r->vars_["y"];
  int x, y;
  param_parser >> x >> y;
  if (x < 0 || y < 0 || x >= 3 || y >= 3)
    co_return Status::BAD_REQUEST;
  if (board->fields[x + y * 3] != ' ')
    co_return Status::BAD_REQUEST;
  board->fields[x + y * 3] = 'x';
  std::string status = CheckBoardState(board);
  if (auto where = ChooseRandomOnBoard(board); where && status == "keep playing") {
    const auto[kX, kY] = *where;
    board->fields[kX + kY * 3] = 'o';
    status = CheckBoardState(board);
    w.Body() << R"({"x":)" << kX << R"(,"y":)" << kY << ",";
  } else {
    w.Body() << "{";
  }
  w.Body() << R"("status":")" << status << R"("})";
  co_return Status::OK;
}

Handle GetBoard(ResponseWriter w, [[maybe_unused]] Request *r, Board *board) {
  w.Body() << "[";
  for (int i = 0; i < 9; ++i) {
    w.Body() << '"' << board->fields[i] << '"' << (i != 8 ? ',' : ']');
  }
  co_return Status::OK;
}

Handle ResetBoard(ResponseWriter w, Request *r, Board *board) {
  if (auto values = r->GetHeader().Get("Authorization"); !values.empty() && values.front() == "Basic dXNlcjp1c2Vy") {
    std::fill(board->fields.begin(), board->fields.end(), ' ');
    co_return Status::OK;
  }
  w.Header()["WWW-Authenticate"].emplace_back(R"(Basic realm="User Visible Realm", charset="UTF-8")");
  co_return Status::UNAUTHORIZED;
}

std::string CheckBoardState(Board *board) {
  auto check = [board](std::vector<std::tuple<int, int>> positions) -> char {
    const auto[kX1, kY1] = positions[0];
    const auto[kX2, kY2] = positions[1];
    const auto[kX3, kY3] = positions[2];
    if (board->fields[kX1 + kY1 * 3] == board->fields[kX2 + kY2 * 3]
        && board->fields[kX2 + kY2 * 3] == board->fields[kX3 + kY3 * 3])
      return board->fields[kX1 + kY1 * 3];
    return ' ';
  };
  std::map<char, std::string> status = {{'x', "victory"}, {'o', "defeat"}};
  if (auto res = check({{0, 0}, {1, 0}, {2, 0}}); res != ' ')
    return status[res];
  if (auto res = check({{0, 1}, {1, 1}, {2, 1}}); res != ' ')
    return status[res];
  if (auto res = check({{0, 2}, {1, 2}, {2, 2}}); res != ' ')
    return status[res];
  if (auto res = check({{0, 0}, {0, 1}, {0, 2}}); res != ' ')
    return status[res];
  if (auto res = check({{1, 0}, {1, 1}, {1, 2}}); res != ' ')
    return status[res];
  if (auto res = check({{2, 0}, {2, 1}, {2, 2}}); res != ' ')
    return status[res];
  if (auto res = check({{0, 0}, {1, 1}, {2, 2}}); res != ' ')
    return status[res];
  if (auto res = check({{0, 2}, {1, 1}, {2, 0}}); res != ' ')
    return status[res];
  return "keep playing";
}

std::optional<std::tuple<int, int>> ChooseRandomOnBoard(Board *board) {
  std::vector<std::tuple<int, int>> available;
  for (int x = 0; x < 3; ++x)
    for (int y = 0; y < 3; ++y)
      if (board->fields[x + y * 3] == ' ')
        available.emplace_back(x, y);
  if (!available.empty()) {
    std::random_device r;
    std::default_random_engine e(r());
    std::uniform_int_distribution<int> uniform_dist(0, available.size() - 1);
    return available[uniform_dist(e)];
  }
  return std::nullopt;
}
