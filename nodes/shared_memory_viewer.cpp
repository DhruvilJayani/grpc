#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <iomanip>

// Same structure as in shared_memory.hpp
struct SharedData {
    int counter;
    int last_target;
    int message_history[10];
    int history_size;
    int history_index;
};

int main() {
    // Open the shared memory
    int fd = shm_open("/grpc_shm", O_RDONLY, 0666);
    if (fd == -1) {
        std::cerr << "Failed to open shared memory. Is the process running?" << std::endl;
        return 1;
    }

    // Map the shared memory
    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData), 
                                                    PROT_READ, MAP_SHARED,
                                                    fd, 0));
    if (data == MAP_FAILED) {
        std::cerr << "Failed to map shared memory" << std::endl;
        close(fd);
        return 1;
    }

    // Print the shared memory contents
    std::cout << "\n=== Shared Memory Contents ===" << std::endl;
    std::cout << "Total messages processed: " << data->counter << std::endl;
    std::cout << "Last target node: " << (data->last_target == -1 ? "None" : 
                                        (data->last_target == 0 ? "Node C" : "Node D")) << std::endl;
    
    std::cout << "\nMessage History:" << std::endl;
    if (data->history_size == 0) {
        std::cout << "No messages in history" << std::endl;
    } else {
        // Start from the oldest message
        int start_idx = (data->history_index - data->history_size + 10) % 10;
        for (int i = 0; i < data->history_size; i++) {
            int idx = (start_idx + i) % 10;
            std::cout << "Message " << i + 1 << ": " << data->message_history[idx] << std::endl;
        }
    }

    // Cleanup
    munmap(data, sizeof(SharedData));
    close(fd);

    return 0;
} 