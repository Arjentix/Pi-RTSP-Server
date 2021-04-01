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

#include "camera.h"

#include <unistd.h>

namespace {

/**
 * @brief Initialize and open camera
 *
 * @return Opened camera object
 */
raspicam::RaspiCam OpenCamera() {
  raspicam::RaspiCam camera;
  camera.setFormat(raspicam::RASPICAM_FORMAT_RGB);
  camera.setFrameRate(10);
  if (!camera.open()) {
    throw CameraOpeningError("Can't open camera");
  }

  sleep(3);
  return camera;
}

} // namespace

CameraOpeningError::CameraOpeningError(std::string_view message) :
std::runtime_error(message.data()) {}

raspicam::RaspiCam &Camera::GetInstance() {
  static raspicam::RaspiCam camera = OpenCamera();
  return camera;
}
