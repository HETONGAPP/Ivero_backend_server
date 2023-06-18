#include "ivero_backend_server/webrtc_json_parser.h"

namespace ivero_backend_server {

bool WebrtcJsonParser::GetStringFromJsonObject(const Json::Value &in,
                                               const std::string &k,
                                               std::string *out) {
  Json::Value x;
  return WebrtcJsonParser::GetValueFromJsonObject(in, k, &x) &&
         WebrtcJsonParser::GetStringFromJson(x, out);
}

bool WebrtcJsonParser::GetValueFromJsonObject(const Json::Value &in,
                                              const std::string &k,
                                              Json::Value *out) {
  if (!in.isObject() || !in.isMember(k)) {
    return false;
  }

  *out = in[k];
  return true;
}

bool WebrtcJsonParser::GetStringFromJson(const Json::Value &in,
                                         std::string *out) {
  if (!in.isString()) {
    if (in.isBool()) {
      *out = WebrtcJsonParser::ToString(in.asBool());
    } else if (in.isInt()) {
      *out = WebrtcJsonParser::ToString(in.asInt());
    } else if (in.isUInt()) {
      *out = WebrtcJsonParser::ToString(in.asUInt());
    } else if (in.isDouble()) {
      *out = WebrtcJsonParser::ToString(in.asDouble());
    } else {
      return false;
    }
  } else {
    *out = in.asString();
  }
  return true;
}

bool WebrtcJsonParser::GetIntFromJsonObject(const Json::Value &in,
                                            const std::string &k, int *out) {
  Json::Value x;
  return WebrtcJsonParser::GetValueFromJsonObject(in, k, &x) &&
         WebrtcJsonParser::GetIntFromJson(x, out);
}

bool WebrtcJsonParser::GetIntFromJson(const Json::Value &in, int *out) {
  bool ret;
  if (!in.isString()) {
    ret = in.isConvertibleTo(Json::intValue);
    if (ret) {
      *out = in.asInt();
    }
  } else {
    long val; // NOLINT
    const char *c_str = in.asCString();
    char *end_ptr;
    errno = 0;
    val = strtol(c_str, &end_ptr, 10); // NOLINT
    ret = (end_ptr != c_str && *end_ptr == '\0' && !errno &&
           val >= std::numeric_limits<int>::min() &&
           val <= std::numeric_limits<int>::max());
    *out = val;
  }
  return ret;
}

std::string WebrtcJsonParser::ToString(const bool b) {
  return b ? "true" : "false";
}
std::string WebrtcJsonParser::ToString(const int s) {
  char buf[32];
  const int len = std::snprintf(&buf[0], 32, "%d", s);
  return std::string(&buf[0], len);
}
std::string WebrtcJsonParser::ToString(const unsigned int s) {
  char buf[32];
  const int len = std::snprintf(&buf[0], 32, "%u", s);
  return std::string(&buf[0], len);
}

std::string WebrtcJsonParser::ToString(const double d) {
  char buf[32];
  const int len = std::snprintf(&buf[0], 32, "%g", d);
  return std::string(&buf[0], len);
}

} // namespace ivero_backend_server
