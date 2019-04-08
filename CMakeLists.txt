cmake_minimum_required(VERSION 3.13)
project(steamlink)

set(CMAKE_CXX_STANDARD 14)

include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/networking/discovery.proto)

add_executable(steamlink src/main.cpp src/networking/Discovery.cpp src/networking/Discovery.h ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(steamlink ${PROTOBUF_LIBRARY})