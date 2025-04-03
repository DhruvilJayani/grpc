cmake_minimum_required(VERSION 3.10)
project(grpc_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Add subdirectories
add_subdirectory(nodes/nodeA)
add_subdirectory(nodes/nodeB)
add_subdirectory(nodes/nodeC)
add_subdirectory(nodes/nodeD)
add_subdirectory(nodes/nodeE) 