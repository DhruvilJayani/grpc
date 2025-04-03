#!/bin/bash

# Function to run a command in a new terminal window
run_in_terminal() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        osascript <<EOF
tell application "Terminal"
    do script "cd '$2' && $3"
    activate
end tell
EOF
    else
        # Linux
        gnome-terminal -- bash -c "$2; exec bash"
    fi
}

# Store the original directory
ORIGINAL_DIR="$(pwd)"

# Start Node B (C++ server)
echo "Starting Node B..."
run_in_terminal "NodeB" "$ORIGINAL_DIR/nodes/nodeB" "./build/server 0.0.0.0:50051 user1"

# Wait for Node B to start
sleep 2

# Start Node E (C++ server) - Start Node E before Node C
echo "Starting Node E..."
run_in_terminal "NodeE" "$ORIGINAL_DIR/nodes/nodeE" "./build/server 0.0.0.0:50055"

# Wait for Node E to start
sleep 2

# Start Node C (C++ server)
echo "Starting Node C..."
run_in_terminal "NodeC" "$ORIGINAL_DIR/nodes/nodeC" "./build/server"

# Wait for Node C to start
sleep 2

# Start Node D (C++ server)
echo "Starting Node D..."
run_in_terminal "NodeD" "$ORIGINAL_DIR/nodes/nodeD" "./build/server"

# Wait for Node D to start
sleep 2

# Start Node A (Python client)
echo "Starting Node A..."
run_in_terminal "NodeA" "$ORIGINAL_DIR/nodes/nodeA" "source ../../venv/bin/activate && python3 client.py"

echo "All nodes started in separate terminal windows."
echo "You can now see the output from each node in its own window."