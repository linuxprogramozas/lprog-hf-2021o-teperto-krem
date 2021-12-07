/*! @file
 * @author Ondrejó András
 * @date 2021.12.02
 */
#pragma once
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <sstream>

namespace tepertokrem {
struct StreamContainer;
namespace http {

/**
 * A valasz testet es fejlecet ebbe lehet visszairni
 */
class ResponseWriter {
 public:
  ResponseWriter() = default;
  explicit ResponseWriter(StreamContainer *stream);
  ResponseWriter(const ResponseWriter&) = default;
  ResponseWriter &operator=(const ResponseWriter&) = default;

  /**
   * Valasz testenek egy stringstream tokeletes
   * Szoveget ebbe a legegyszerubb irni, de ha kell binaris adatot is elfogat
   */
  using BodyType = std::stringstream;

  /**
   * A valasz fejlecben egy kulcshoz (string) tobb ertek is tartozhat (vector<string>)
   * A kulcsnal mindegy, hogy kis vagy nagybetu
   */
  using HeaderType = std::map<std::string, std::vector<std::string>>;

  BodyType &Body();
  HeaderType &Header();
  StreamContainer *Stream();
 private:
  struct ResponseWriterData {
    StreamContainer *stream;
    BodyType body = {};
    HeaderType header = {};
  };
  std::shared_ptr<ResponseWriterData> data_;
};
}
}