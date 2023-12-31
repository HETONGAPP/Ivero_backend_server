#ifndef __WEBRTC_WEB_SERVER_H__
#define __WEBRTC_WEB_SERVER_H__

#include <string>

namespace ivero_backend_server {

class SignalingChannel {
public:
  virtual ~SignalingChannel();
  virtual void sendPingMessage() = 0;
  virtual void sendTextMessage(const std::string &message) = 0;
};

class MessageHandler {
public:
  enum Type { TEXT, PONG, CLOSE };

  MessageHandler();
  virtual ~MessageHandler();

  virtual void handle_message(Type type, const std::string &raw) = 0;
};

typedef MessageHandler *(*SignalingChannelCallback)(void *, SignalingChannel *);

class WebrtcWebServer {
protected:
public:
  static WebrtcWebServer *create(int port, SignalingChannelCallback callback,
                                 void *data);

  WebrtcWebServer();
  virtual ~WebrtcWebServer();

  virtual void run() = 0;
  virtual void stop() = 0;
};

} // namespace ivero_backend_server

#endif // __WEBRTC_WEB_SERVER_H__