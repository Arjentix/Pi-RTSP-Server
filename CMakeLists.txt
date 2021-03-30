cmake_minimum_required(VERSION 3.13)
project(pi-rtsp-server)

set(SRC_DIR src)

set(BUILD_FLAGS "-Wall -Wextra -pedantic")

set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/camera.cpp
    ${SRC_DIR}/exception/camera_opening_error.cpp
    ${SRC_DIR}/rtsp/request.cpp
    ${SRC_DIR}/rtsp/response.cpp
    ${SRC_DIR}/sdp/session_description.cpp
    ${SRC_DIR}/sock/exception.cpp
    ${SRC_DIR}/sock/socket.cpp
    ${SRC_DIR}/sock/server_socket.cpp
    ${SRC_DIR}/sock/client_socket.cpp
    ${SRC_DIR}/processing/request_dispatcher.cpp
    ${SRC_DIR}/processing/servlet.cpp
    ${SRC_DIR}/processing/servlets/jpeg.cpp
    ${SRC_DIR}/rtp/serializable.cpp
    ${SRC_DIR}/rtp/mjpeg/packet.cpp
    ${SRC_DIR}/rtp/packet.cpp
)

# Disabling OpenCv searching for raspicam build
set(BUILD_CV OFF)
add_subdirectory(external/raspicam)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(JPEGTURBO REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${SRC_DIR}
    external/raspicam/src/
    ${JPEGTURBO_INCLUDE_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    raspicam
    ${JPEGTURBO_LIBRARIES}
)

set_target_properties(${CMAKE_PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_FLAGS ${BUILD_FLAGS}
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
