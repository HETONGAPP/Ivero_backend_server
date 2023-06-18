#include "webrtc/rtc_base/bind.h"
#include <boost/enable_shared_from_this.hpp>
#include <chrono>
#include <ivero_backend_server/video_capture.h>
#include <opencv2/opencv.hpp>
namespace ivero_backend_server {

VideoCapturer::VideoCapturer() : impl_(new VideoCapturerImpl()) {}

VideoCapturer::~VideoCapturer() {
  Stop(); // Make sure were stopped so callbacks stop
}

void VideoCapturer::Start() { impl_->Start(this); }

void VideoCapturer::Stop() { impl_->Stop(); }

void VideoCapturer::imageCallback(zmq::socket_t &subscriber) {

  // int frameCount = 0;
  // auto startTime = std::chrono::steady_clock::now();
  while (!impl_->flag_) {

    zmq::message_t zmq_msg;
    subscriber.recv(&zmq_msg);

    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    int64_t system_time_us =
        std::chrono::duration_cast<std::chrono::microseconds>(currentTime)
            .count();

    cv::Mat bgr(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, zmq_msg.data(),
                cv::Mat::AUTO_STEP);

    cv::Rect roi;
    int out_width = IMAGE_WIDTH;
    int out_height = IMAGE_HEIGHT;

    cv::Mat yuv;
    cv::cvtColor(bgr, yuv, cv::COLOR_BGR2YUV_I420);
    uint8_t *y = yuv.data;
    uint8_t *u = y + (out_width * out_height);
    uint8_t *v = u + (out_width * out_height) / 4;

    std::shared_ptr<webrtc::VideoFrame> frame =
        std::make_shared<webrtc::VideoFrame>(
            webrtc::I420Buffer::Copy(out_width, out_height, y, out_width, u,
                                     out_width / 2, v, out_width / 2),
            webrtc::kVideoRotation_0, system_time_us);

    OnFrame(*frame);
  }
  subscriber.close();
}

bool VideoCapturer::is_screencast() const { return false; }

absl::optional<bool> VideoCapturer::needs_denoising() const { return false; }
webrtc::MediaSourceInterface::SourceState VideoCapturer::state() const {
  return webrtc::MediaSourceInterface::kLive;
}
bool VideoCapturer::remote() const { return false; }

VideoCapturerImpl::VideoCapturerImpl()
    : capturer_(nullptr), CAM_IP_ADDRESS_(STREAM_PORT) {
  context_ = std::make_shared<zmq::context_t>(1);
  subscriber_ = std::make_shared<zmq::socket_t>(*context_, ZMQ_SUB);
}

void VideoCapturerImpl::Start(VideoCapturer *capturer) {

  // std::unique_lock<std::mutex> lock(state_mutex_);

  IVERO_SERVER_PRINT_INFO("Realsense camera pipeline has been started");

  subscriber_->connect(
      CAM_IP_ADDRESS_); // change the ip or the port if which has been occupied!

  subscriber_->setsockopt(ZMQ_SUBSCRIBE, "", 0);

  callbackThread_ = std::make_shared<std::thread>(
      &VideoCapturerImpl::imageCallback, this, std::ref(*subscriber_));

  capturer_ = capturer;
}

void VideoCapturerImpl::Stop() {

  /* this function will block the camera to exit the current thread (ToDo fix
   * the issue) std::unique_lock<std::mutex> lock(state_mutex_);*/

  flag_ = true;

  callbackThread_->join();

  if (capturer_ == nullptr)
    return;

  capturer_ = nullptr;

  IVERO_SERVER_PRINT_INFO("Realsense camera pipeline has been fininshed.");
}

void VideoCapturerImpl::imageCallback(zmq::socket_t &subscriber) {

  // test
  // std::unique_lock<std::mutex> lock(state_mutex_);

  if (capturer_ == nullptr) {
    IVERO_SERVER_PRINT_ERROR("No capture stream has been created!");
    return;
  }

  rs2::context ctx;
  rs2::device_list devices = ctx.query_devices();

  if (devices.size() == 0) {
    IVERO_SERVER_PRINT_ERROR("No RealSense cameras detected.");
    return;
  }
  if (IsProcessRunning(RGB_PROCESS)) {
    IVERO_SERVER_PRINT_RGB("Publishing the RGB image ...");

    capturer_->imageCallback(subscriber);
  } else {
    IVERO_SERVER_PRINT_WARNING("Camera process is not running");
  }
  return;
}
} // namespace ivero_backend_server
