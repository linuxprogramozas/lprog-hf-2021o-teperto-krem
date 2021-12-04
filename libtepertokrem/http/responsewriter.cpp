/*! @file
 * @author Ondrejó András
 * @date 2021.12.02
 */
#include "responsewriter.hpp"

namespace tepertokrem::http {
ResponseWriter::ResponseWriter(StreamContainer *stream): data_(new ResponseWriterData{.stream = stream}) {}

ResponseWriter::BodyType &ResponseWriter::Body() {
  return data_->body;
}

ResponseWriter::HeaderType &ResponseWriter::Header() {
  return data_->header;
}

StreamContainer *ResponseWriter::Stream() {
  return data_->stream;
}

}