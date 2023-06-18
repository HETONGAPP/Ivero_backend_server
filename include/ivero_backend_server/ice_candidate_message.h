#ifndef __ICE_CANDIDATE_MESSAGE_H__
#define __ICE_CANDIDATE_MESSAGE_H__

#include <ivero_backend_server/webrtc_message.h>
#include <jsoncpp/json/json.h>
#include <webrtc/api/jsep.h>

namespace ivero_backend_server {

class IceCandidateMessage {
public:
  static std::string kIceCandidateType;
  static std::string kSdpMidFieldName;
  static std::string kSdpMlineIndexFieldName;
  static std::string kCandidateFieldName;

  static bool isIceCandidate(const Json::Value &message_json);

  bool fromJson(const Json::Value &message_json);
  bool fromIceCandidate(const webrtc::IceCandidateInterface &ice_candidate);

  webrtc::IceCandidateInterface *createIceCandidate();
  std::string toJson();

  IceCandidateMessage();

  std::string sdp_mid;
  int sdp_mline_index;
  std::string candidate;
};

} // namespace ivero_backend_server
#endif // __ICE_CANDIDATE_MESSAGE_H__