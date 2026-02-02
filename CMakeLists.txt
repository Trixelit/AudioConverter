cmake_minimum_required(VERSION 4.1)
project(AudioConverter)

set(CMAKE_CXX_STANDARD 20)

set(FFMPEG_ROOT "C:/ffmpeg")

include_directories(
        ${FFMPEG_ROOT}/include
)

link_directories(
        ${FFMPEG_ROOT}/lib
)

add_executable(AudioConverter main.cpp)

target_link_libraries(AudioConverter avformat avcodec avutil swresample)