#ifndef __VIDEO_CAPTURE_H__
#define __VIDEO_CAPTURE_H__

#include <atomic>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <ivero_backend_server/utility.h>
#include <librealsense2/rs.hpp>
#include <mutex>
#include <thread>
#include <webrtc/api/video/i420_buffer.h>
#include <webrtc/api/video/video_source_interface.h>
#include <webrtc/media/base/adapted_video_track_source.h>
#include <webrtc/modules/video_capture/video_capture.h>
#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/rtc_base/event.h>
#include <webrtc/rtc_base/thread.h>
#include <zmq.hpp>

namespace ivero_backend_server {
class VideoCapturerImpl;
class VideoCapturer : public rtc::AdaptedVideoTrackSource {
public:
  VideoCapturer();
  ~VideoCapturer() override;
  void imageCallback(zmq::socket_t &subscriber);
  void Start();

  void Stop();

  bool is_screencast() const override;
  absl::optional<bool> needs_denoising() const override;
  void SetState(webrtc::MediaSourceInterface::SourceState state);
  webrtc::MediaSourceInterface::SourceState state() const override;
  bool remote() const override;
  bool stopCapture_ = false;
  // std::atomic<bool> stopCapture_(false);

private:
  RTC_DISALLOW_COPY_AND_ASSIGN(VideoCapturer);
  boost::shared_ptr<VideoCapturerImpl> impl_;
};

class VideoCapturerImpl
    : public boost::enable_shared_from_this<VideoCapturerImpl> {
public:
  VideoCapturerImpl();

  void imageCallback(zmq::socket_t &subscriber);

  void Start(VideoCapturer *capturer);
  void Stop();
  bool flag_ = false;

private:
  RTC_DISALLOW_COPY_AND_ASSIGN(VideoCapturerImpl);
  std::string CAM_IP_ADDRESS_;
  std::mutex state_mutex_;
  VideoCapturer *capturer_;
  rs2::pipeline *pipeline_;
  std::shared_ptr<std::thread> callbackThread_;
  std::shared_ptr<zmq::context_t> context_;
  std::shared_ptr<zmq::socket_t> subscriber_;
};

} // namespace ivero_backend_server

#endif // __VIDEO_CAPTURE_H__