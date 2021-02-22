cmake_minimum_required(VERSION 3.13)
project(pi-rtsp-server)

set(SOURCES
    src/main.cpp
)

# Disabling OpenCv searching for raspicam build
set(BUILD_CV OFF)

# Making off_t type use 64 bit size to fix tinyalsa build
#add_compile_definitions(_FILE_OFFSET_BITS=64)

add_subdirectory(external/raspicam)
#add_subdirectory(external/tinyalsa)


add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    external/raspicam/src/
#    external/tinyalsa/include/
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    raspicam
#    tinyalsa
)

set_target_properties(${CMAKE_PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
