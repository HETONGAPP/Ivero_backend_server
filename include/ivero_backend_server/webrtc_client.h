#ifndef __WEBRTC_CLIENT_H__
#define __WEBRTC_CLIENT_H__

#include <webrtc/api/audio_codecs/builtin_audio_decoder_factory.h>
#include <webrtc/api/audio_codecs/builtin_audio_encoder_factory.h>
#include <webrtc/api/audio_options.h>
#include <webrtc/api/create_peerconnection_factory.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/pc/peer_connection_factory.h>

#include <ivero_backend_server/configure_message.h>
#include <webrtc/media/base/adapted_video_track_source.h>
#include <webrtc/media/engine/internal_decoder_factory.h>
#include <webrtc/media/engine/internal_encoder_factory.h>
#include <webrtc/media/engine/multiplex_codec_factory.h>
#include <webrtc/rtc_base/thread.h>

#include <atomic>
#include <ivero_backend_server/webrtc_web_server.h>
namespace ivero_backend_server {
class WebrtcClient;

typedef std::shared_ptr<WebrtcClient> WebrtcClientPtr;
typedef std::weak_ptr<WebrtcClient> WebrtcClientWeakPtr;

class WebrtcClientObserverProxy
    : public webrtc::PeerConnectionObserver,
      public webrtc::CreateSessionDescriptionObserver,
      public webrtc::DataChannelObserver {
public:
  WebrtcClientObserverProxy(WebrtcClientWeakPtr client_weak);
  // void OnBufferedAmountChange(uint64_t sent_data_size) override;
  void OnSuccess(webrtc::SessionDescriptionInterface *) override;
  void OnFailure(webrtc::RTCError error) override;
  //   void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>)
  //   override; void
  //       OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>)
  //       override;
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer &buffer) override;
  void OnRenegotiationNeeded() override;
  void OnIceCandidate(const webrtc::IceCandidateInterface *) override;
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState) override;
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState) override;
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState) override;
  void OnIceCandidatesRemoved(
      const std::vector<cricket::Candidate> &candidates) override;

private:
  WebrtcClientWeakPtr client_weak_;
};

class MessageHandlerImpl;
class WebrtcClient {
public:
  WebrtcClient(SignalingChannel *signaling_channel);
  ~WebrtcClient();
  MessageHandler *createMessageHandler();

  void init(std::shared_ptr<WebrtcClient> &keep_alive_ptr);
  void invalidate();
  bool valid();

private:
  WebrtcClientPtr keep_alive_this_;

  bool initPeerConnection();

  void handle_message(MessageHandler::Type type, const std::string &message);

  void OnSessionDescriptionSuccess(webrtc::SessionDescriptionInterface *);

  void OnSessionDescriptionFailure(const std::string &);

  void OnIceCandidate(const webrtc::IceCandidateInterface *);

  void OnPointCloudStreaning();

  void
  OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);

  void OnStateChange();

  void OnMessage(const webrtc::DataBuffer &buffer);

  std::atomic<bool> shouldExit{false};

  std::unique_ptr<SignalingChannel> signaling_channel_;

  rtc::Thread *signaling_thread_;
  std::unique_ptr<rtc::Thread> worker_thread_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;

  rtc::scoped_refptr<WebrtcClientObserverProxy> webrtc_observer_proxy_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

  std::map<std::string, std::map<std::string, std::string>> expected_streams_;

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;

  std::shared_ptr<std::thread> callbackThread_;
  // rclcpp::TimerBase::SharedPtr ping_timer_;

  friend WebrtcClientObserverProxy;
  friend MessageHandlerImpl;
};

} // namespace ivero_backend_server

#endif // __WEBRTC_CLIENT_H__