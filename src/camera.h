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

#include <stdexcept>

// No need to show warnings from external library header
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "raspicam.h"
#pragma GCC diagnostic pop

/**
 * @brief Exception indicating error during camera opening
 */
class CameraOpeningError : public std::runtime_error {
 public:
  CameraOpeningError(std::string_view message);
};

/**
 * @brief Singleton for camera accessing
 */
class Camera {
 public:
  Camera() = delete;
  Camera(const Camera &) = delete;
  Camera &operator=(const Camera &) = delete;

  /**
   * @brief Get instance of camera object
   *
   * @return Camera object from raspicam library
   */
  static raspicam::RaspiCam &GetInstance();
};
