# Pi RTSP Server

## Description

Raspberry Pi RTSP Server for live video translation. It is making your Raspberry works just like an IP Camera!

Video is captured using *Pi Camera*, packed in *JPEG* and sent over the RTSP/RTP protocol.

Tested with `Rasbperry Pi 3 B+, Raspbian GNU/Linux 10 (buster), libjpeg-turbo0 v1:1.5.2-2+deb10u1`.

## RTSP

Supported RTSP methods:

1. `OPTIONS`
2. `DESCRIBE`
3. `SETUP`
4. `PLAY`
5. `TEARDOWN`

Video will be placed on `rtsp://yourip:5544/jpeg` url

### Limitations

1. Only one client, who is playing video, at a time
2. Only 10 fps or lower
3. No RTCP support
4. No config support
5. No file logging support
6. No authorization support
7. No encryption support

## Dependencies

This project has 3 dependencies:

1. [Raspberry Pi Camera](https://www.raspberrypi.org/products/camera-module-v2/) — camera device especially for Raspberry Pi. Don't forget to turn it on in the `raspi-config`!
2. [raspicam](https://github.com/rmsalinas/raspicam) — allows to grab image from Raspberry Pi Camera. It comes as a git submodule
3. [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo) — allows working with *JPEG* data. I am using it to compress raw data from camera to *JPEG* encoded image. This one you should install with `sudo apt install libturbojpeg0-dev`

## Download & Build

Make sure you have installed **libjpeg-turbo**👆

```bash
git clone --recursive https://github.com/Arjentix/Pi-RTSP-Server
cd Pi-RTSP-Server
mkdir build
cd build
cmake ..
cmake --build . -j8
```

## Known bugs

1. Keeps sending data if client disconnected without `TEARDOWN` method sent
2. Sends full JPEG image without cutting extra info

