#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct SharedData {
    int counter;
    int last_target;
    int message_history[100];
    int messages_to_b[100];
    int messages_to_c[100];
    int messages_to_d[100];
    int history_size;
    int b_size;
    int c_size;
    int d_size;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <user_id>" << std::endl;
        return 1;
    }

    std::string user_id = argv[1];
    std::string filename = "nodeB/" + user_id + "_shared_data.bin";

    // Open the file
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    // Map the file into memory
    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData), PROT_READ, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        std::cerr << "Failed to map file" << std::endl;
        close(fd);
        return 1;
    }

    // Convert arrays to vectors for JSON
    std::vector<int> history(data->message_history, data->message_history + data->history_size);
    std::vector<int> to_b(data->messages_to_b, data->messages_to_b + data->b_size);
    std::vector<int> to_c(data->messages_to_c, data->messages_to_c + data->c_size);
    std::vector<int> to_d(data->messages_to_d, data->messages_to_d + data->d_size);

    // Convert to JSON for pretty printing
    json j;
    j["counter"] = data->counter;
    j["last_target"] = data->last_target;
    j["message_history"] = history;
    j["messages_to_b"] = to_b;
    j["messages_to_c"] = to_c;
    j["messages_to_d"] = to_d;

    // Print the data
    std::cout << "\nShared Memory Contents:" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << "Total messages processed: " << j["counter"] << std::endl;
    std::cout << "Last target node: " << (j["last_target"] == 0 ? "C" : "D") << std::endl;
    std::cout << "\nMessage History: " << j["message_history"].dump() << std::endl;
    std::cout << "\nMessages forwarded to Node B: " << j["messages_to_b"].dump() << std::endl;
    std::cout << "Messages forwarded to Node C: " << j["messages_to_c"].dump() << std::endl;
    std::cout << "Messages forwarded to Node D: " << j["messages_to_d"].dump() << std::endl;
    std::cout << "======================\n" << std::endl;

    // Cleanup
    munmap(data, sizeof(SharedData));
    close(fd);

    return 0;
} 