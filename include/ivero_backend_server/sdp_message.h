#ifndef __SDP_MESSAGE_H__
#define __SDP_MESSAGE_H__

#include <ivero_backend_server/webrtc_message.h>
#include <jsoncpp/json/json.h>
#include <webrtc/api/jsep.h>

namespace ivero_backend_server {
class SdpMessage {
public:
  static std::string kSdpFieldName;
  static std::string kSdpOfferType;
  static std::string kSdpAnswerType;

  static bool isSdpOffer(const Json::Value &message_json);
  static bool isSdpAnswer(const Json::Value &message_json);

  bool fromJson(const Json::Value &message_json);
  bool fromSessionDescription(
      const webrtc::SessionDescriptionInterface &description);

  webrtc::SessionDescriptionInterface *createSessionDescription();
  std::string toJson();

  SdpMessage();

  std::string type;
  std::string sdp;
};
} // namespace ivero_backend_server
#endif // __SDP_MESSAGE_H__