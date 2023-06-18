#ifndef __WEBRTC_JSON_PARSER_H__
#define __WEBRTC_JSON_PARSER_H__

#include <jsoncpp/json/json.h>
#include <limits>

namespace ivero_backend_server {
class WebrtcJsonParser {
public:
  static bool getType(const Json::Value &message_json, std::string *type);

  static bool GetStringFromJsonObject(const Json::Value &in,
                                      const std::string &k, std::string *out);

  static bool GetValueFromJsonObject(const Json::Value &in,
                                     const std::string &k, Json::Value *out);
  static bool GetStringFromJson(const Json::Value &in, std::string *out);

  static bool GetIntFromJson(const Json::Value &in, int *out);
  static bool GetIntFromJsonObject(const Json::Value &in, const std::string &k,
                                   int *out);
  static std::string ToString(const bool b);
  static std::string ToString(const double b);
  static std::string ToString(const int b);
  static std::string ToString(const unsigned int b);
}; // namespace ivero_backend_server
} // namespace ivero_backend_server
#endif // __WEBRTC_JSON_PARSER_H__
