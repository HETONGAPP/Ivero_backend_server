#ifndef __WEBRTC_MESSAGE_H__
#define __WEBRTC_MESSAGE_H__

#include <ivero_backend_server/webrtc_json_parser.h>
#include <jsoncpp/json/json.h>

namespace ivero_backend_server {
class WebrtcMessage {
public:
  static std::string kMessageTypeFieldName;

  static bool isType(const Json::Value &message_json, const std::string &type);
  static bool getType(const Json::Value &message_json, std::string *type);
};
} // namespace ivero_backend_server

#endif // __WEBRTC_MESSAGE_H__