#include <async_web_server_cpp/http_reply.hpp>
#include <async_web_server_cpp/http_server.hpp>
#include <async_web_server_cpp/websocket_connection.hpp>
#include <async_web_server_cpp/websocket_request_handler.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <chrono>
#include <ivero_backend_server/video_capture.h>
#include <ivero_backend_server/webrtc_web_server.h>

namespace ivero_backend_server {

SignalingChannel::~SignalingChannel() {}

class SignalingChannelImpl : public SignalingChannel {
public:
  SignalingChannelImpl(async_web_server_cpp::WebsocketConnectionPtr websocket)
      : websocket_(websocket) {}
  void sendPingMessage() { websocket_->sendPingMessage(); }
  void sendTextMessage(const std::string &message) {
    websocket_->sendTextMessage(message);
  }

private:
  async_web_server_cpp::WebsocketConnectionPtr websocket_;
};

MessageHandler::MessageHandler() {}
MessageHandler::~MessageHandler() {}

WebrtcWebServer::WebrtcWebServer() {}
WebrtcWebServer::~WebrtcWebServer() {}

class WebrtcWebServerImpl : public WebrtcWebServer {
public:
  WebrtcWebServerImpl(int port, SignalingChannelCallback callback, void *data)
      : WebrtcWebServer(),
        handler_group_(async_web_server_cpp::HttpReply::stock_reply(
            async_web_server_cpp::HttpReply::not_found)),
        callback_(callback), data_(data) {
    std::vector<async_web_server_cpp::HttpHeader> any_origin_headers;

    std::string Ivero_backend_server = WORKSPACE;

    any_origin_headers.push_back(
        async_web_server_cpp::HttpHeader("Access-Control-Allow-Origin", "*"));

    handler_group_.addHandlerForPath(
        "/index",
        async_web_server_cpp::HttpReply::from_file(
            async_web_server_cpp::HttpReply::ok, "text/html",
            Ivero_backend_server + "/web/index.html", any_origin_headers));

    handler_group_.addHandlerForPath(
        "/",
        async_web_server_cpp::HttpReply::from_file(
            async_web_server_cpp::HttpReply::ok, "text/html",
            Ivero_backend_server + "/web/viewer.html", any_origin_headers));

    handler_group_.addHandlerForPath(
        "/webrtc", async_web_server_cpp::WebsocketHttpRequestHandler(std::bind(
                       &WebrtcWebServerImpl::handle_webrtc_websocket, this,
                       std::placeholders::_1, std::placeholders::_2)));

    handler_group_.addHandlerForPath(
        "/.+", async_web_server_cpp::HttpReply::from_filesystem(
                   async_web_server_cpp::HttpReply::ok, "/",
                   Ivero_backend_server + "/web", false, any_origin_headers));

    server_.reset(new async_web_server_cpp::HttpServer(
        "0.0.0.0", boost::lexical_cast<std::string>(port),
        std::bind(connection_logger, handler_group_, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3,
                  std::placeholders::_4),
        1));
  }

  ~WebrtcWebServerImpl() { stop(); }

  void run() {
    server_->run();
    IVERO_SERVER_PRINT_INFO("Waiting For connections...");
  }

  void stop() { server_->stop(); }

private:
  class WebsocketMessageHandlerWrapper {
  public:
    WebsocketMessageHandlerWrapper(MessageHandler *callback)
        : callback_(callback) {}
    void operator()(const async_web_server_cpp::WebsocketMessage &message) {
      MessageHandler::Type type;

      if (message.type == async_web_server_cpp::WebsocketMessage::type_text) {
        type = MessageHandler::TEXT;
      } else if (message.type ==
                 async_web_server_cpp::WebsocketMessage::type_pong) {
        type = MessageHandler::PONG;
      } else if (message.type ==
                 async_web_server_cpp::WebsocketMessage::type_close) {
        type = MessageHandler::CLOSE;
      } else {
        // ROS_WARN_STREAM("Unexpected websocket message type: " << message.type
        // << ": " << message.content);
        IVERO_SERVER_PRINT_ERROR("Unexpected websocket message type");
        return;
      }
      callback_->handle_message(type, message.content);
    }

  private:
    boost::shared_ptr<MessageHandler> callback_;
  };

  async_web_server_cpp::WebsocketConnection::MessageHandler
  handle_webrtc_websocket(
      const async_web_server_cpp::HttpRequest &request,
      async_web_server_cpp::WebsocketConnectionPtr websocket) {
    return WebsocketMessageHandlerWrapper(
        callback_(data_, new SignalingChannelImpl(websocket)));
  }

  static bool
  connection_logger(async_web_server_cpp::HttpServerRequestHandler forward,
                    const async_web_server_cpp::HttpRequest &request,
                    async_web_server_cpp::HttpConnectionPtr connection,
                    const char *begin, const char *end) {
    boost::asio::ip::tcp::socket &socket = connection->socket();
    boost::asio::ip::tcp::endpoint remoteEndpoint = socket.remote_endpoint();
    std::string remoteAddress = remoteEndpoint.address().to_string();
    std::string requestMethod = request.method;
    std::string requestUri = request.uri;

    // std::cout << remoteAddress << ": " << requestMethod << " " << requestUri
    //           << std::endl;

    try {
      return forward(request, connection, begin, end);
    } catch (std::exception &e) {
      IVERO_SERVER_PRINT_ERROR("Error Handling Request: " << e.what());
    }

    return false;
  }

  boost::shared_ptr<async_web_server_cpp::HttpServer> server_;
  async_web_server_cpp::HttpRequestHandlerGroup handler_group_;

  SignalingChannelCallback callback_;
  void *data_;
};

WebrtcWebServer *WebrtcWebServer::create(int port,
                                         SignalingChannelCallback callback,
                                         void *data) {

  return new WebrtcWebServerImpl(port, callback, data);
}

} // namespace ivero_backend_server
