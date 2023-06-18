#include "ivero_backend_server/webrtc_message.h"

namespace ivero_backend_server {

bool WebrtcMessage::isType(const Json::Value &message_json,
                           const std::string &expected_type) {
  std::string type;
  if (getType(message_json, &type))
    return expected_type.compare(type) == 0;
  return false;
}

bool WebrtcMessage::getType(const Json::Value &message_json,
                            std::string *type) {
  return WebrtcJsonParser::GetStringFromJsonObject(message_json,
                                                   kMessageTypeFieldName, type);
}

std::string WebrtcMessage::kMessageTypeFieldName = "type";

} // namespace ivero_backend_server