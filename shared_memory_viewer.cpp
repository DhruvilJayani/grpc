#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

struct SharedData {
    int counter;
    int last_target;
    int message_history[100];
    int messages_to_b[100];
    int messages_to_c[100];
    int messages_to_d[100];
    int messages_to_e[100];
    int history_size;
    int b_size;
    int c_size;
    int d_size;
    int e_size;
    int last_even_id;
    int last_odd_id;
};

void printSharedMemory(const std::string& user_id) {
    std::string filename = user_id + "_shared_data.bin";
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error: Could not open shared memory file for " << user_id << std::endl;
        return;
    }

    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData), PROT_READ, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        close(fd);
        std::cerr << "Error: Could not map shared memory for " << user_id << std::endl;
        return;
    }

    std::cout << "\nShared Memory Contents:\n";
    std::cout << "======================\n";
    std::cout << "Total messages processed: " << data->counter << "\n";
    std::cout << "Last target node: " << (data->last_target == 0 ? "C" : "D") << "\n";
    std::cout << "Last even ID (sent to D): " << data->last_even_id << "\n";
    std::cout << "Last odd ID (sent to C): " << data->last_odd_id << "\n\n";

    std::cout << "Message History: [";
    for (int i = 0; i < data->history_size; ++i) {
        if (i > 0) std::cout << ",";
        std::cout << data->message_history[i];
    }
    std::cout << "]\n\n";

    std::cout << "Messages forwarded to Node B: [";
    for (int i = 0; i < data->b_size; ++i) {
        if (i > 0) std::cout << ",";
        std::cout << data->messages_to_b[i];
    }
    std::cout << "]\n";

    std::cout << "Messages forwarded to Node C: [";
    for (int i = 0; i < data->c_size; ++i) {
        if (i > 0) std::cout << ",";
        std::cout << data->messages_to_c[i];
    }
    std::cout << "]\n";

    std::cout << "Messages forwarded to Node D: [";
    for (int i = 0; i < data->d_size; ++i) {
        if (i > 0) std::cout << ",";
        std::cout << data->messages_to_d[i];
    }
    std::cout << "]\n";

    std::cout << "Messages forwarded to Node E: [";
    for (int i = 0; i < data->e_size; ++i) {
        if (i > 0) std::cout << ",";
        std::cout << data->messages_to_e[i];
    }
    std::cout << "]\n";
    std::cout << "======================\n";

    munmap(data, sizeof(SharedData));
    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <user_id>\n";
        std::cerr << "Example: " << argv[0] << " user1\n";
        return 1;
    }

    std::string user_id = argv[1];
    printSharedMemory(user_id);

    return 0;
} 