#!/bin/bash

# Store the original directory
ORIGINAL_DIR=$(pwd)
echo "Original Directory: $ORIGINAL_DIR"

# Build Node B (C++ server)
echo "Building Node B (C++ server)..."
cd nodes/nodeB
mkdir -p build
cd build
cmake ..
make

# Build Node C (C++ server)
echo "Building Node C (C++ server)..."
cd ../../nodeC
mkdir -p build
cd build
cmake ..
make

# Build Node D (C++ server)
echo "Building Node D (C++ server)..."
cd ../../nodeD
mkdir -p build
cd build
cmake ..
make

# Build Node E (C++ server)
echo "Building Node E (C++ server)..."
cd ../../nodeE
mkdir -p build
cd build
cmake ..
make

# Generate Python protobuf files for Node A
echo "Generating Python protobuf files for Node A..."
cd "$ORIGINAL_DIR/.."

# Now we use the correct path to the protos directory
python3 -m grpc_tools.protoc \
    -I=./protos \
    --python_out=./nodes/nodeA \
    --grpc_python_out=./nodes/nodeA \
    ./protos/data.proto

echo "Build complete!"
