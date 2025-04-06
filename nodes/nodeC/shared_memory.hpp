#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <limits.h>

using json = nlohmann::json;

// Default capacities for each dynamic array (adjust as needed)
const int DEFAULT_HISTORY_CAPACITY = 1000000;
const int DEFAULT_B_CAPACITY = 1000000;
const int DEFAULT_C_CAPACITY = 1000000;
const int DEFAULT_D_CAPACITY = 1000000;
const int DEFAULT_E_CAPACITY = 1000000;

// Header structure stored at the beginning of the shared memory file.
// This holds counters, sizes, and the capacities of each dynamic array.
struct SharedDataHeader {
    int counter;
    int last_target;      // e.g., 0 for Node C, 1 for Node D, etc.
    int history_size;
    int b_size;
    int c_size;
    int d_size;
    int e_size;
    int last_even_id;     // Track last even ID sent to Node D
    int last_odd_id;      // Track last odd ID sent to Node C
    int history_capacity;
    int b_capacity;
    int c_capacity;
    int d_capacity;
    int e_capacity;
};

class SharedMemory {
private:
    std::string filename_;
    int fd_;
    void* mapped_;
    size_t size_;

    // Pointers into the mapped memory.
    SharedDataHeader* header_;
    int* message_history_;
    int* messages_to_b_;
    int* messages_to_c_;
    int* messages_to_d_;
    int* messages_to_e_;

    void initialize() {
        // Get current working directory to build an absolute path.
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            throw std::runtime_error("Failed to get current working directory");
        }
        // Create absolute path to shared memory file (placed in the parent directory).
        std::string abs_path = std::string(cwd) + "/../" + filename_;
        std::cout << "NodeC: Using shared memory file: " << abs_path << std::endl;

        // Check if the file already exists.
        struct stat st;
        bool file_exists = (stat(abs_path.c_str(), &st) == 0);
        std::cout << "NodeC: File exists: " << (file_exists ? "yes" : "no") << std::endl;

        // Open (or create) the file.
        fd_ = open(abs_path.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ == -1) {
            std::cerr << "NodeC: Failed to open file: " << abs_path << " (errno: " << errno << ")" << std::endl;
            throw std::runtime_error("Failed to open file");
        }

        // Calculate total size: header plus dynamic arrays.
        size_ = sizeof(SharedDataHeader) +
                (DEFAULT_HISTORY_CAPACITY +
                 DEFAULT_B_CAPACITY +
                 DEFAULT_C_CAPACITY +
                 DEFAULT_D_CAPACITY +
                 DEFAULT_E_CAPACITY) * sizeof(int);

        // If the file is new, set its size.
        if (!file_exists) {
            if (ftruncate(fd_, size_) == -1) {
                close(fd_);
                throw std::runtime_error("Failed to set file size");
            }
        } else {
            size_ = st.st_size;
        }

        // Map the file into memory.
        mapped_ = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mapped_ == MAP_FAILED) {
            close(fd_);
            throw std::runtime_error("Failed to map file");
        }

        // The header is stored at the beginning of the mapped memory.
        header_ = static_cast<SharedDataHeader*>(mapped_);
        // Initialize the header if the file is new.
        if (!file_exists) {
            header_->counter = 0;
            header_->last_target = -1;
            header_->history_size = 0;
            header_->b_size = 0;
            header_->c_size = 0;
            header_->d_size = 0;
            header_->e_size = 0;
            header_->last_even_id = 0;
            header_->last_odd_id = 0;
            header_->history_capacity = DEFAULT_HISTORY_CAPACITY;
            header_->b_capacity = DEFAULT_B_CAPACITY;
            header_->c_capacity = DEFAULT_C_CAPACITY;
            header_->d_capacity = DEFAULT_D_CAPACITY;
            header_->e_capacity = DEFAULT_E_CAPACITY;
            msync(mapped_, size_, MS_SYNC);
        }

        // Set pointers to the dynamic arrays by offset from the header.
        char* base = static_cast<char*>(mapped_);
        message_history_ = reinterpret_cast<int*>(base + sizeof(SharedDataHeader));
        messages_to_b_ = message_history_ + header_->history_capacity;
        messages_to_c_ = messages_to_b_ + header_->b_capacity;
        messages_to_d_ = messages_to_c_ + header_->c_capacity;
        messages_to_e_ = messages_to_d_ + header_->d_capacity;
    }

public:
    SharedMemory(const std::string& user_id) {
        filename_ = user_id + "_shared_data.bin";
        initialize();
    }

    ~SharedMemory() {
        if (mapped_ != MAP_FAILED) {
            munmap(mapped_, size_);
        }
        if (fd_ != -1) {
            close(fd_);
        }
    }

    void incrementCounter() {
        header_->counter++;
        msync(mapped_, size_, MS_SYNC);
    }

    int getCounter() const {
        return header_->counter;
    }

    void updateMessageHistory(int message_id) {
        if (header_->history_size < header_->history_capacity) {
            message_history_[header_->history_size] = message_id;
            header_->history_size++;
        } else {
            // If full, shift elements left (as a circular buffer) and store the new ID.
            memmove(message_history_, message_history_ + 1, (header_->history_capacity - 1) * sizeof(int));
            message_history_[header_->history_capacity - 1] = message_id;
        }
        msync(mapped_, size_, MS_SYNC);
    }

    void addMessageToNode(int message_id, int node) {
        switch (node) {
            case 0: // Node B
                if (header_->b_size < header_->b_capacity) {
                    messages_to_b_[header_->b_size] = message_id;
                    header_->b_size++;
                }
                break;
            case 1: // Node C
                if (header_->c_size < header_->c_capacity) {
                    messages_to_c_[header_->c_size] = message_id;
                    header_->c_size++;
                    header_->last_odd_id = message_id;
                }
                break;
            case 2: // Node D
                if (header_->d_size < header_->d_capacity) {
                    messages_to_d_[header_->d_size] = message_id;
                    header_->d_size++;
                    header_->last_even_id = message_id;
                }
                break;
            case 3: // Node E
                if (header_->e_size < header_->e_capacity) {
                    messages_to_e_[header_->e_size] = message_id;
                    header_->e_size++;
                }
                break;
        }
        msync(mapped_, size_, MS_SYNC);
    }

    void setLastTarget(int target) {
        header_->last_target = target;
        msync(mapped_, size_, MS_SYNC);
    }

    json toJson() const {
        json j;
        j["counter"] = header_->counter;
        j["last_target"] = header_->last_target;
        j["history_size"] = header_->history_size;
        j["b_size"] = header_->b_size;
        j["c_size"] = header_->c_size;
        j["d_size"] = header_->d_size;
        j["e_size"] = header_->e_size;
        j["last_even_id"] = header_->last_even_id;
        j["last_odd_id"] = header_->last_odd_id;

        j["message_history"] = json::array();
        for (int i = 0; i < header_->history_size; ++i) {
            j["message_history"].push_back(message_history_[i]);
        }

        j["messages_to_b"] = json::array();
        for (int i = 0; i < header_->b_size; ++i) {
            j["messages_to_b"].push_back(messages_to_b_[i]);
        }

        j["messages_to_c"] = json::array();
        for (int i = 0; i < header_->c_size; ++i) {
            j["messages_to_c"].push_back(messages_to_c_[i]);
        }

        j["messages_to_d"] = json::array();
        for (int i = 0; i < header_->d_size; ++i) {
            j["messages_to_d"].push_back(messages_to_d_[i]);
        }

        j["messages_to_e"] = json::array();
        for (int i = 0; i < header_->e_size; ++i) {
            j["messages_to_e"].push_back(messages_to_e_[i]);
        }

        return j;
    }
};

#endif // SHARED_MEMORY_HPP
