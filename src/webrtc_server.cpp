#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/ssl_adapter.h"
#include <iostream>
#include <ivero_backend_server/webrtc_server.h>

namespace ivero_backend_server {

MessageHandler *
WebrtcServer_handle_new_signaling_channel(void *_this,
                                          SignalingChannel *channel) {
  return ((WebrtcServer *)_this)
      ->signaling_thread_->Invoke<MessageHandler *>(
          RTC_FROM_HERE, rtc::Bind(&WebrtcServer::handle_new_signaling_channel,
                                   (WebrtcServer *)_this, channel));
}

WebrtcServer::WebrtcServer(const int port) {
  rtc::InitializeSSL();

  signaling_thread_ = rtc::Thread::CreateWithSocketServer();

  signaling_thread_->Start();

  server_.reset(WebrtcWebServer::create(
      port, &WebrtcServer_handle_new_signaling_channel, this));
}

void WebrtcServer::cleanupWebrtcClient(WebrtcClient *client) {
  {
    std::unique_lock<std::mutex> lock(clients_mutex_);
    clients_.erase(client);
    delete client; // delete while holding the lock so that we do not exit
                   // before were done
  }
  shutdown_cv_.notify_all();
}

MessageHandler *
WebrtcServer::handle_new_signaling_channel(SignalingChannel *channel) {
  auto client = std::make_shared<WebrtcClient>(channel);

  // TODO: Handle cleanup std::bind(&WebrtcRosServer::cleanupWebrtcClient, this,
  // std::placeholders::_1));
  // hold a shared ptr until the object is initialized (holds a ptr to itself)
  client->init(client);
  {
    std::unique_lock<std::mutex> lock(clients_mutex_);
    clients_[client.get()] = WebrtcClientWeakPtr(client);
  }
  return client->createMessageHandler();
}

WebrtcServer::~WebrtcServer() {
  stop();

  // Send all clients messages to shutdown, cannot call dispose of share ptr
  // while holding clients_mutex_ It will deadlock if it is the last shared_ptr
  // because it will try to remove it from the client list
  std::vector<WebrtcClientWeakPtr> to_invalidate;
  {
    std::unique_lock<std::mutex> lock(clients_mutex_);
    for (auto &client_entry : clients_) {
      to_invalidate.push_back(client_entry.second);
    }
  }
  for (WebrtcClientWeakPtr &client_weak : to_invalidate) {
    std::shared_ptr<WebrtcClient> client = client_weak.lock();
    if (client)
      client->invalidate();
  }

  // Wait for all our clients to shown
  {
    std::unique_lock<std::mutex> lock(clients_mutex_);
    shutdown_cv_.wait(lock, [this] { return this->clients_.size() == 0; });
  }

  rtc::CleanupSSL();
}

void WebrtcServer::run() {
  IVERO_SERVER_PRINT_INFO("Running the Webrtc Server...")
  server_->run();
}

void WebrtcServer::stop() {
  IVERO_SERVER_PRINT_WARNING("Stopping the Webrtc Server...")
  server_->stop();
}

} // namespace ivero_backend_server
