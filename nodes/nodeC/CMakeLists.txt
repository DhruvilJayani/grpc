cmake_minimum_required(VERSION 3.10)
project(grpc_server)

# Set policies
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0144 NEW)  # Accept lowercase _ROOT variables

# Architecture settings
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)

# Find Protobuf installation
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

# Find gRPC installation
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

add_executable(server
    server.cpp
    data.pb.cc
    data.pb.h
    data.grpc.pb.cc
    data.grpc.pb.h
)

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(server PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
    Boost::system
    nlohmann_json::nlohmann_json
)

# Copy config.json to build directory
configure_file(${CMAKE_SOURCE_DIR}/config.json
              ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)