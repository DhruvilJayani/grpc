cmake_minimum_required(VERSION 3.10)
project(shared_memory_viewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the shared memory viewer executable
add_executable(shared_memory_viewer shared_memory_viewer.cpp)

# Link against required libraries
target_link_libraries(shared_memory_viewer rt) 