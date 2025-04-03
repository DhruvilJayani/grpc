cmake_minimum_required(VERSION 3.10)

# Generate protobuf files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../protos/data.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
protobuf_generate_grpc_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_FILES})

# Create a library for the generated protobuf files
add_library(data_proto
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${GRPC_SRCS}
    ${GRPC_HDRS}
)

target_link_libraries(data_proto
    PRIVATE
    gRPC::grpc++
    gRPC::grpc
    protobuf::libprotobuf
) 