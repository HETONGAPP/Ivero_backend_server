#include <ivero_backend_server/webrtc_server.h>

int main(int argc, char **argv) {

  std::signal(SIGINT, ivero_backend_server::signalHandler);

  ivero_backend_server::IP_PORT =
      std::stoi(ivero_backend_server::getValueFromFile("IP_PORT", 2));

  ivero_backend_server::printPattern("Ivero Backend Server Start");

  ivero_backend_server::WebrtcServer server(ivero_backend_server::IP_PORT);

  std::thread serverThread([&server]() { server.run(); });

  ivero_backend_server::RUN();

  serverThread.join();

  return 0;
}