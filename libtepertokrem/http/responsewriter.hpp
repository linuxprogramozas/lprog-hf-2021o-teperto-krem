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
class ResponseWriter {
 public:
  ResponseWriter() = default;
  explicit ResponseWriter(StreamContainer *stream);
  ResponseWriter(const ResponseWriter&) = default;
  ResponseWriter &operator=(const ResponseWriter&) = default;

  using BodyType = std::stringstream;
  using HeaderType = std::map<std::string, std::vector<std::string>>;

  BodyType &Body();
  HeaderType &Header();
  StreamContainer *Stream();
 private:
  struct ResponseWriterData {
    StreamContainer *stream;
    BodyType body;
    HeaderType header;
  };
  std::shared_ptr<ResponseWriterData> data_;
};
}
}