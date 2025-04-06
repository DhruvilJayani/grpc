#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// The header layout must match the one in shared_memory.hpp.
struct SharedDataHeader {
    int counter;
    int last_target;
    int history_size;
    int b_size;
    int c_size;
    int d_size;
    int e_size;
    int last_even_id;
    int last_odd_id;
    int history_capacity;
    int b_capacity;
    int c_capacity;
    int d_capacity;
    int e_capacity;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <user_id>" << std::endl;
        return 1;
    }

    std::string user_id = argv[1];
    std::string filename = user_id + "_shared_data.bin";

    std::cout << "Shared Memory Viewer: Using file: " << filename << std::endl;

    // Open the shared memory file.
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Failed to open file: " << filename << " (errno: " << errno << ")" << std::endl;
        return 1;
    }

    // First, read the header.
    SharedDataHeader header;
    ssize_t headerSize = sizeof(SharedDataHeader);
    if (read(fd, &header, headerSize) != headerSize) {
        std::cerr << "Failed to read header from file." << std::endl;
        close(fd);
        return 1;
    }

    // Calculate total size.
    size_t totalSize = sizeof(SharedDataHeader) +
        (header.history_capacity + header.b_capacity +
         header.c_capacity + header.d_capacity +
         header.e_capacity) * sizeof(int);

    // Map the entire file.
    void* mapped = mmap(nullptr, totalSize, PROT_READ, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "Failed to map file" << std::endl;
        close(fd);
        return 1;
    }

    SharedDataHeader* headerPtr = static_cast<SharedDataHeader*>(mapped);

    // Get pointers to the dynamic arrays.
    char* base = static_cast<char*>(mapped);
    int* message_history = reinterpret_cast<int*>(base + sizeof(SharedDataHeader));
    int* messages_to_b = message_history + headerPtr->history_capacity;
    int* messages_to_c = messages_to_b + headerPtr->b_capacity;
    int* messages_to_d = messages_to_c + headerPtr->c_capacity;
    int* messages_to_e = messages_to_d + headerPtr->d_capacity;

    // Convert arrays to vectors for JSON.
    std::vector<int> history(message_history, message_history + headerPtr->history_size);
    std::vector<int> to_b(messages_to_b, messages_to_b + headerPtr->b_size);
    std::vector<int> to_c(messages_to_c, messages_to_c + headerPtr->c_size);
    std::vector<int> to_d(messages_to_d, messages_to_d + headerPtr->d_size);
    std::vector<int> to_e(messages_to_e, messages_to_e + headerPtr->e_size);

    // Build JSON for pretty printing.
    json j;
    j["counter"] = headerPtr->counter;
    j["last_target"] = headerPtr->last_target;
    j["last_even_id"] = headerPtr->last_even_id;
    j["last_odd_id"] = headerPtr->last_odd_id;
    j["history_size"] = headerPtr->history_size;
    j["b_size"] = headerPtr->b_size;
    j["c_size"] = headerPtr->c_size;
    j["d_size"] = headerPtr->d_size;
    j["e_size"] = headerPtr->e_size;

    j["message_history"] = json::array();
    for (int i = 0; i < headerPtr->history_size; ++i) {
        j["message_history"].push_back(message_history[i]);
    }

    j["messages_to_b"] = json::array();
    for (int i = 0; i < headerPtr->b_size; ++i) {
        j["messages_to_b"].push_back(messages_to_b[i]);
    }

    j["messages_to_c"] = json::array();
    for (int i = 0; i < headerPtr->c_size; ++i) {
        j["messages_to_c"].push_back(messages_to_c[i]);
    }

    j["messages_to_d"] = json::array();
    for (int i = 0; i < headerPtr->d_size; ++i) {
        j["messages_to_d"].push_back(messages_to_d[i]);
    }

    j["messages_to_e"] = json::array();
    for (int i = 0; i < headerPtr->e_size; ++i) {
        j["messages_to_e"].push_back(messages_to_e[i]);
    }

    std::cout << "\nShared Memory Contents:" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << "Total messages processed: " << j["counter"] << std::endl;
    std::cout << "Last target node: " << (j["last_target"] == 0 ? "C" : "D") << std::endl;
    std::cout << "Last even ID (sent to D): " << j["last_even_id"] << std::endl;
    std::cout << "Last odd ID (sent to C): " << j["last_odd_id"] << std::endl;
    std::cout << "\nMessage History: " << j["message_history"].dump() << std::endl;
    std::cout << "\nMessages forwarded to Node B: " << j["messages_to_b"].dump() << std::endl;
    std::cout << "Messages forwarded to Node C: " << j["messages_to_c"].dump() << std::endl;
    std::cout << "Messages forwarded to Node D: " << j["messages_to_d"].dump() << std::endl;
    std::cout << "Messages forwarded to Node E: " << j["messages_to_e"].dump() << std::endl;
    std::cout << "======================\n" << std::endl;

    munmap(mapped, totalSize);
    close(fd);

    return 0;
}
