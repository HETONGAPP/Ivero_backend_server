#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iostream>
#include <ivero_backend_server/ice_candidate_message.h>
#include <ivero_backend_server/sdp_message.h>
#include <ivero_backend_server/video_capture.h>
#include <ivero_backend_server/webrtc_client.h>
#include <ivero_backend_server/webrtc_message.h>
#include <librealsense2/rs.hpp>
#include <memory>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <typeinfo>
#include <unistd.h>
#include <webrtc/api/data_channel_interface.h>
#include <webrtc/api/video/video_source_interface.h>
#include <webrtc/rtc_base/bind.h>
#include <zmq.hpp>

using namespace std::chrono_literals;
using std::placeholders::_1;
constexpr size_t MAX_BUFFERED_AMOUNT = 262144;

namespace ivero_backend_server {

WebrtcClientObserverProxy::WebrtcClientObserverProxy(
    WebrtcClientWeakPtr client_weak)
    : client_weak_(client_weak) {}

void WebrtcClientObserverProxy::OnSuccess(
    webrtc::SessionDescriptionInterface *description) {
  WebrtcClientPtr client = client_weak_.lock();
  if (client)
    client->OnSessionDescriptionSuccess(description);
}

void WebrtcClientObserverProxy::OnFailure(webrtc::RTCError error) {
  WebrtcClientPtr client = client_weak_.lock();
  if (client)
    client->OnSessionDescriptionFailure(error.message());
}

void WebrtcClientObserverProxy::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  WebrtcClientPtr client = client_weak_.lock();
  if (client)
    client->OnDataChannel(data_channel);
}
void WebrtcClientObserverProxy::OnStateChange() {
  WebrtcClientPtr client = client_weak_.lock();
  if (client)
    client->OnStateChange();
}
void WebrtcClientObserverProxy::OnMessage(const webrtc::DataBuffer &buffer) {
  WebrtcClientPtr client = client_weak_.lock();
  if (client)
    client->OnMessage(buffer);
}
void WebrtcClientObserverProxy::OnRenegotiationNeeded() {}

void WebrtcClientObserverProxy::OnIceCandidate(
    const webrtc::IceCandidateInterface *candidate) {
  WebrtcClientPtr client = client_weak_.lock();
  if (client)
    client->OnIceCandidate(candidate);
}

void WebrtcClientObserverProxy::OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState) {}

void WebrtcClientObserverProxy::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState) {}

void WebrtcClientObserverProxy::OnIceCandidatesRemoved(
    const std::vector<cricket::Candidate> &) {}

void WebrtcClientObserverProxy::OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState) {}

WebrtcClient::WebrtcClient(SignalingChannel *signaling_channel)
    : signaling_channel_(signaling_channel),
      signaling_thread_(rtc::Thread::Current()),
      worker_thread_(rtc::Thread::CreateWithSocketServer()) {
  worker_thread_->Start();

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
      worker_thread_.get(), worker_thread_.get(), worker_thread_.get(), nullptr,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(),
      std::unique_ptr<webrtc::VideoEncoderFactory>(
          new webrtc::MultiplexEncoderFactory(
              std::make_unique<webrtc::InternalEncoderFactory>())),
      std::unique_ptr<webrtc::VideoDecoderFactory>(
          new webrtc::MultiplexDecoderFactory(
              std::make_unique<webrtc::InternalDecoderFactory>())),
      nullptr, nullptr);
  if (!peer_connection_factory_.get()) {
    IVERO_SERVER_PRINT_WARNING("Could not create peer connection factory");
    invalidate();
    return;
  }
}

WebrtcClient::~WebrtcClient() {
  shouldExit = true;
  // Wait for the detached thread to finish

  if (valid()) {
    IVERO_SERVER_PRINT_ERROR(
        "WebrtcClient destructor should only be called once it's invalidated");
  }
  IVERO_SERVER_PRINT_WARNING("Destroying Webrtc Client...");
}

void WebrtcClient::init(std::shared_ptr<WebrtcClient> &keep_alive_ptr) {
  keep_alive_this_ = keep_alive_ptr;
}

void WebrtcClient::invalidate() { keep_alive_this_.reset(); }

bool WebrtcClient::valid() { return keep_alive_this_ != nullptr; }

bool WebrtcClient::initPeerConnection() {

  if (!valid()) {
    IVERO_SERVER_PRINT_ERROR("Tried to initialize invalidated webrtc client");
    return false;
  }

  if (!peer_connection_) {
    webrtc::PeerConnectionInterface::RTCConfiguration config;

    WebrtcClientWeakPtr weak_this(keep_alive_this_);
    webrtc_observer_proxy_ =
        new rtc::RefCountedObject<WebrtcClientObserverProxy>(weak_this);
    peer_connection_ = peer_connection_factory_->CreatePeerConnection(
        config, nullptr, nullptr, webrtc_observer_proxy_.get());
    if (!peer_connection_.get()) {
      IVERO_SERVER_PRINT_WARNING("Could not create peer connection");
      invalidate();
      return false;
    }
    webrtc::DataChannelInit data_channel_config;

    data_channel_config.ordered = true;
    data_channel_config.maxRetransmitTime = 0;
    data_channel_config.maxRetransmits = 0;

    data_channel_ =
        peer_connection_->CreateDataChannel("Pointcloud", &data_channel_config);

    data_channel_->RegisterObserver(webrtc_observer_proxy_.get());

    return true;
  } else {
    return true;
  }
}

class MessageHandlerImpl : public MessageHandler {
public:
  MessageHandlerImpl(WebrtcClientWeakPtr weak_this) : weak_this_(weak_this) {}
  void handle_message(MessageHandler::Type type, const std::string &raw) {
    WebrtcClientPtr _this = weak_this_.lock();
    if (_this)
      _this->signaling_thread_->Invoke<void>(
          RTC_FROM_HERE,
          rtc::Bind(&WebrtcClient::handle_message, _this.get(), type, raw));
  }

private:
  WebrtcClientWeakPtr weak_this_;
};

MessageHandler *WebrtcClient::createMessageHandler() {
  return new MessageHandlerImpl(keep_alive_this_);
}

class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
public:
  virtual void OnSuccess() {
    //// RCLCPP_DEBUG(_node->get_logger(),__FUNCTION__);
  }
  virtual void OnFailure(webrtc::RTCError error) {
    // RCLCPP_WARN_STREAM(nh_->get_logger(), __FUNCTION__ << " " << error);
  }

protected:
  DummySetSessionDescriptionObserver() {}
  ~DummySetSessionDescriptionObserver() {}
};

static bool parseUri(const std::string &uri, std::string *scheme_name,
                     std::string *path) {
  size_t split = uri.find_first_of(':');
  if (split == std::string::npos)
    return false;
  *scheme_name = uri.substr(0, split);
  if (uri.length() > split + 1)
    *path = uri.substr(split + 1, uri.length() - split - 1);
  else
    *path = "";
  return true;
}

void WebrtcClient::handle_message(MessageHandler::Type type,
                                  const std::string &raw) {

  if (type == MessageHandler::TEXT) {
    Json::Reader reader;
    Json::Value message_json;
    // IVERO_SERVER_PRINT_INFO("JSON:" + raw);
    IVERO_SERVER_PRINT_INFO("JSON: parsing message ...");
    if (!reader.parse(raw, message_json)) {
      IVERO_SERVER_PRINT_INFO("Could not parse message: " << raw);
      invalidate();
      return;
    }
    if (ConfigureMessage::isConfigure(message_json)) {
      ConfigureMessage message;
      if (!message.fromJson(message_json)) {
        IVERO_SERVER_PRINT_WARNING("Can't parse received configure message.");
        return;
      }

      if (!initPeerConnection()) {
        IVERO_SERVER_PRINT_WARNING("Failed to initialize peer connection");
        return;
      }
      for (const ConfigureAction &action : message.actions) {
        // Macro that simply checks if a key is specified and will ignore the
        // action is not specified
#define FIND_PROPERTY_OR_CONTINUE(key, name)                                   \
  if (action.properties.find(key) == action.properties.end()) {                \
    IVERO_SERVER_PRINT_INFO("No " << #name << " specified");                   \
    continue;                                                                  \
  }                                                                            \
  std::string name = action.properties.at(key)
        // END OF MACRO

        if (action.type == ConfigureAction::kAddStreamActionName) {
          FIND_PROPERTY_OR_CONTINUE("id", stream_id);

          rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
              peer_connection_factory_->CreateLocalMediaStream(stream_id);

          if (!peer_connection_->AddStream(stream)) {
            IVERO_SERVER_PRINT_WARNING(
                "Adding stream to PeerConnection failed");
            continue;
          }
        } else if (action.type == ConfigureAction::kRemoveStreamActionName) {
          FIND_PROPERTY_OR_CONTINUE("id", stream_id);

          rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
              peer_connection_factory_->CreateLocalMediaStream(stream_id);

          if (!stream) {
            IVERO_SERVER_PRINT_INFO("Stream not found with id: " << stream_id);
            continue;
          }
          peer_connection_->RemoveStream(stream);
        } else if (action.type == ConfigureAction::kAddVideoTrackActionName) {
          FIND_PROPERTY_OR_CONTINUE("stream_id", stream_id);
          FIND_PROPERTY_OR_CONTINUE("id", track_id);
          FIND_PROPERTY_OR_CONTINUE("src", src);

          std::string video_type;
          std::string video_path;
          if (!parseUri(src, &video_type, &video_path)) {
            IVERO_SERVER_PRINT_WARNING("Invalid URI: " << src);
            continue;
          }

          webrtc::MediaStreamInterface *stream =
              peer_connection_->local_streams()->find(stream_id);
          if (!stream) {
            IVERO_SERVER_PRINT_WARNING(
                "Stream not found with id: " << stream_id);
            continue;
          }

          if (video_type == "Realsense_Image") {
            IVERO_SERVER_PRINT_INFO("Getting the Realsense Camera Image");
            // IVERO_SERVER_PRINT_INFO(track_id);
            rtc::scoped_refptr<VideoCapturer> capturer(
                new rtc::RefCountedObject<VideoCapturer>());
            rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
                peer_connection_factory_->CreateVideoTrack(track_id, capturer));
            IVERO_SERVER_PRINT_INFO("Adding the video track...");
            stream->AddTrack(video_track);
            IVERO_SERVER_PRINT_INFO("video track has been added");
            capturer->Start();

          } else {
            IVERO_SERVER_PRINT_WARNING(
                "Unknown video source type: " << video_type);
          }

        } else if (action.type == ConfigureAction::kAddAudioTrackActionName) {
          FIND_PROPERTY_OR_CONTINUE("stream_id", stream_id);
          FIND_PROPERTY_OR_CONTINUE("id", track_id);
          FIND_PROPERTY_OR_CONTINUE("src", src);

          std::string audio_type;
          std::string audio_path;
          if (!parseUri(src, &audio_type, &audio_path)) {
            IVERO_SERVER_PRINT_ERROR("Invalid URI: " << src);
            continue;
          }

          webrtc::MediaStreamInterface *stream =
              peer_connection_->local_streams()->find(stream_id);

          // std::shared_ptr<webrtc::MediaStreamInterface> stream =
          //     std::make_shared<webrtc::MediaStreamInterface>(
          //         peer_connection_->local_streams()->find(stream_id));
          if (!stream) {
            IVERO_SERVER_PRINT_ERROR("Stream not found with id: " << stream_id);
            continue;
          }

          if (audio_type == "local") {
            cricket::AudioOptions options;
            rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
                peer_connection_factory_->CreateAudioTrack(
                    track_id,
                    peer_connection_factory_->CreateAudioSource(options)));
            stream->AddTrack(audio_track);
          } else {
            IVERO_SERVER_PRINT_INFO(
                "Unknown video source type: " << audio_type);
          }

        } else if (action.type == ConfigureAction::kExpectStreamActionName) {
          FIND_PROPERTY_OR_CONTINUE("id", stream_id);
          if (expected_streams_.find(stream_id) != expected_streams_.end()) {
            IVERO_SERVER_PRINT_INFO("Stream id: " << stream_id
                                                  << " is already expected");
            continue;
          }
          expected_streams_[stream_id] = std::map<std::string, std::string>();
        } else if (action.type ==
                   ConfigureAction::kExpectVideoTrackActionName) {
          FIND_PROPERTY_OR_CONTINUE("stream_id", stream_id);
          FIND_PROPERTY_OR_CONTINUE("id", track_id);
          FIND_PROPERTY_OR_CONTINUE("dest", dest);

          if (expected_streams_.find(stream_id) == expected_streams_.end()) {
            IVERO_SERVER_PRINT_INFO("Stream id: " << stream_id
                                                  << " is not expected");
            continue;
          }
          if (expected_streams_[stream_id].find(track_id) !=
              expected_streams_[stream_id].end()) {
            IVERO_SERVER_PRINT_INFO("Track id: "
                                    << track_id
                                    << " is already expected in stream id: "
                                    << stream_id);
            continue;
          }
          expected_streams_[stream_id][track_id] = dest;
        } else {
          IVERO_SERVER_PRINT_INFO(
              "Unknown configure action type: " << action.type);
        }
      }
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options(
          0, 0, false, false, false);
      peer_connection_->CreateOffer(webrtc_observer_proxy_.get(), options);
      // TODO check media constraints
    } else if (SdpMessage::isSdpAnswer(message_json)) {
      SdpMessage message;

      if (!message.fromJson(message_json)) {
        IVERO_SERVER_PRINT_WARNING(
            "Can't parse received session description message.");
        return;
      }

      webrtc::SessionDescriptionInterface *session_description(
          message.createSessionDescription());
      if (!session_description) {
        IVERO_SERVER_PRINT_WARNING("Can't create session description");
        return;
      }

      // IVERO_SERVER_PRINT_WARNING(
      //    "Received remote description: " << message.sdp);
      IVERO_SERVER_PRINT_WARNING("Received remote description");
      rtc::scoped_refptr<DummySetSessionDescriptionObserver>
          dummy_set_description_observer(
              new rtc::RefCountedObject<DummySetSessionDescriptionObserver>());
      peer_connection_->SetRemoteDescription(dummy_set_description_observer,
                                             session_description);
    } else if (IceCandidateMessage::isIceCandidate(message_json)) {
      IceCandidateMessage message;
      if (!message.fromJson(message_json)) {
        IVERO_SERVER_PRINT_WARNING(
            "Can't parse received ice candidate message.");
        return;
      }

      std::unique_ptr<webrtc::IceCandidateInterface> candidate(
          message.createIceCandidate());
      if (!candidate.get()) {
        IVERO_SERVER_PRINT_WARNING("Can't parse received candidate message.");
        return;
      }
      if (!peer_connection_->AddIceCandidate(candidate.get())) {
        IVERO_SERVER_PRINT_WARNING("Failed to apply the received candidate");
        return;
      }
      IVERO_SERVER_PRINT_WARNING(
          "Received remote candidate :" << message.toJson());
      return;
    } else {
      std::string message_type;
      WebrtcMessage::getType(message_json, &message_type);
      IVERO_SERVER_PRINT_WARNING("Unexpected message type: " << message_type
                                                             << ": " << raw);
    }
  } else if (type == MessageHandler::PONG) {
    // got a pong from the last ping
  } else if (type == MessageHandler::CLOSE) {
    invalidate();
  } else {
    IVERO_SERVER_PRINT_WARNING(
        "Unexpected signaling message type: " << type << ": " << raw);
  }
}

void WebrtcClient::OnSessionDescriptionSuccess(
    webrtc::SessionDescriptionInterface *description) {
  rtc::scoped_refptr<DummySetSessionDescriptionObserver>
      dummy_set_description_observer(
          new rtc::RefCountedObject<DummySetSessionDescriptionObserver>());
  peer_connection_->SetLocalDescription(dummy_set_description_observer,
                                        description);

  SdpMessage message;
  if (message.fromSessionDescription(*description)) {
    // IVERO_SERVER_PRINT_WARNING("Created local description: " << message.sdp);
    IVERO_SERVER_PRINT_INFO("Created local description");
    signaling_channel_->sendTextMessage(message.toJson());
  } else {
    IVERO_SERVER_PRINT_WARNING("Failed to serialize description");
  }
}
void WebrtcClient::OnSessionDescriptionFailure(const std::string &error) {
  IVERO_SERVER_PRINT_WARNING("Could not create local description: " << error);
  invalidate();
}
void WebrtcClient::OnIceCandidate(
    const webrtc::IceCandidateInterface *candidate) {
  IceCandidateMessage message;
  if (message.fromIceCandidate(*candidate)) {
    IVERO_SERVER_PRINT_INFO("Got local ICE candidate: " << message.toJson());
    signaling_channel_->sendTextMessage(message.toJson());
  } else {
    IVERO_SERVER_PRINT_WARNING("Failed to serialize local candidate");
  }
}

void WebrtcClient::OnPointCloudStreaning() {
  zmq::context_t context(1);
  zmq::socket_t subscriber(context, ZMQ_SUB);
  subscriber.connect(PCL_PORT); // Change the address as needed
  subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

  while (!shouldExit) {

    zmq::message_t zmq_msg;
    subscriber.recv(&zmq_msg);

    std::string json_str(static_cast<char *>(zmq_msg.data()), zmq_msg.size());

    webrtc::DataBuffer dataBuffer(json_str);

    if (shouldExit) {
      IVERO_SERVER_PRINT_INFO("PointCloud channel has been fininshed.");
      break;
    } else {
      data_channel_->Send(dataBuffer);
    }
  }
}
void WebrtcClient::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  IVERO_SERVER_PRINT_DATACHANNEL(
      "Received a new data channel: " << data_channel->label());
  if (IsProcessRunning(PCL_PROCESS)) {
    IVERO_SERVER_PRINT_DATACHANNEL("Publishing the PointCloud ...");
    callbackThread_ = std::make_shared<std::thread>(
        &WebrtcClient::OnPointCloudStreaning, this);
    callbackThread_->detach();
  } else {
    IVERO_SERVER_PRINT_WARNING("PointCloud Process is not running");
  }
}

void WebrtcClient::OnMessage(const webrtc::DataBuffer &buffer) {
  auto message = std::string(buffer.data.data<char>(), buffer.data.size());

  IVERO_SERVER_PRINT_DATACHANNEL("Received the message from remote")
}

void WebrtcClient::OnStateChange() {
  assert(data_channel_);
  if (data_channel_->state() ==
      webrtc::DataChannelInterface::DataState::kOpen) {
    IVERO_SERVER_PRINT_DATACHANNEL("Data Channel state is open");
    std::string callbackMSG =
        "ivero data channel has been built (backend server)";
    webrtc::DataBuffer buf(callbackMSG);
    if (!data_channel_->Send(buf)) {
      IVERO_SERVER_PRINT_ERROR("Call back message send failed");
    }
  }
}

} // namespace ivero_backend_server
