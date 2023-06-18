#ifndef __WEBRTC_SERVER_H__
#define __WEBRTC_SERVER_H__

#include <boost/shared_ptr.hpp>
#include <condition_variable>
#include <ivero_backend_server/video_capture.h>
#include <ivero_backend_server/webrtc_client.h>
#include <ivero_backend_server/webrtc_web_server.h>

namespace ivero_backend_server {

MessageHandler *
WebrtcServer_handle_new_signaling_channel(void *_this,
                                          SignalingChannel *channel);

class WebrtcServer {
public:
  WebrtcServer(const int port);
  ~WebrtcServer();
  void run();
  void stop();

  MessageHandler *handle_new_signaling_channel(SignalingChannel *channel);
  void cleanupWebrtcClient(WebrtcClient *client);

  std::unique_ptr<rtc::Thread> signaling_thread_;

private:
  std::condition_variable shutdown_cv_;
  std::mutex clients_mutex_;
  std::map<WebrtcClient *, WebrtcClientWeakPtr> clients_;

  boost::shared_ptr<ivero_backend_server::WebrtcWebServer> server_;
};

} // namespace ivero_backend_server
#endif // __WEBRTC_SERVER_H__