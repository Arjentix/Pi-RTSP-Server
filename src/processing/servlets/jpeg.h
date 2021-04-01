/*
MIT License

Copyright (c) 2021 Polyakov Daniil Alexandrovich

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "processing/servlet.h"

#include <queue>
#include <utility>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace processing::servlets {

class Jpeg : public Servlet {
 public:
  Jpeg();

  ~Jpeg() override;

  rtsp::Response ServeDescribe(const rtsp::Request &request) override;

  rtsp::Response ServeSetup(const rtsp::Request &request) override;

  rtsp::Response ServePlay(const rtsp::Request &request) override;

  rtsp::Response ServeTeardown(const rtsp::Request &request) override;

 private:
  const std::string kVideoTrackName = "track1"; //!< Name of the video track

  bool client_connected_; //!< True, if one client is playing a video
  bool teardown_; //!< True, if TEARDOWN was requested
  uint32_t session_id_; //!< Session id. Only one session is supported
  std::pair<int, int> client_ports_; //!< Pair of client RTP and RTCP ports
  std::queue<rtsp::Request> play_queue_; //!< Queue of PLAY requests
  std::thread play_worker_; //!< Thread for PLAY requests processing
  bool play_worker_stop_; //!< True, if play_worker_ should stop
  std::mutex play_worker_mutex_; //!< Mutex to interact with play_worker_
  std::condition_variable play_worker_notifier_; //!< Cond. var. to interact with play_worker_

  /**
   * @brief Extract PLAY request from queue and process it. Used in play_worker_
   */
  void PlayWorkerThread();

  /**
   * @brief Directly handle play request
   *
   * @param request PLAY Request to handle
   */
  void HandlePlayRequest(const rtsp::Request &request);

  /**
   * @brief Check if provided session id is valid
   *
   * @param request Request to check
   *
   * @return true If session id is valid
   * @return false In other way
   */
  bool CheckSession(const rtsp::Request &request) const;
};

} // namespace processing::servlets
