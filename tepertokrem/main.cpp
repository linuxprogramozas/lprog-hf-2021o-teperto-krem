/*! @file
 * @author Ondrejó András
 * @date 2021.11.20
 */
#include <iostream>
#include <application/application.hpp>

using Integer = tepertokrem::NamedType<int, struct IntTag>;

using namespace tepertokrem;

int main(int argc, char **argv) {
  auto &app = Application::Instance();
  return app.ListenAndServe(":8080"_ipv4);
}
