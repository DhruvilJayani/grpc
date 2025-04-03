#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>
#include <sys/stat.h>

using json = nlohmann::json;

struct SharedData {
    int counter;
    int last_target;      // 0 for Node C, 1 for Node D
    int message_history[100];
    int messages_to_b[100];
    int messages_to_c[100];
    int messages_to_d[100];
    int messages_to_e[100];  // Added for Node E
    int history_size;
    int b_size;
    int c_size;
    int d_size;
    int e_size;           // Added for Node E
    int last_even_id;     // Track last even ID sent to Node D
    int last_odd_id;      // Track last odd ID sent to Node C
};

class SharedMemory {
private:
    std::string filename_;
    int fd_;
    SharedData* data_;
    size_t size_;

    void initialize() {
        // Get the absolute path to the shared memory file
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            throw std::runtime_error("Failed to get current working directory");
        }
        
        // Create absolute path to shared memory file in nodes directory
        std::string abs_path = std::string(cwd) + "/../" + filename_;
        std::cout << "NodeD: Using shared memory file: " << abs_path << std::endl;
        
        // First check if file exists
        struct stat st;
        bool file_exists = (stat(abs_path.c_str(), &st) == 0);
        std::cout << "NodeD: File exists: " << (file_exists ? "yes" : "no") << std::endl;
        
        // Create or open the file
        fd_ = open(abs_path.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ == -1) {
            std::cerr << "NodeD: Failed to open file: " << abs_path << " (errno: " << errno << ")" << std::endl;
            throw std::runtime_error("Failed to open file");
        }

        // Only set file size if it's a new file
        if (!file_exists) {
            size_ = sizeof(SharedData);
            if (ftruncate(fd_, size_) == -1) {
                close(fd_);
                throw std::runtime_error("Failed to set file size");
            }
        } else {
            size_ = st.st_size;
        }

        // Map the file into memory
        data_ = static_cast<SharedData*>(mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        if (data_ == MAP_FAILED) {
            close(fd_);
            throw std::runtime_error("Failed to map file");
        }

        // Only initialize if this is a new file
        if (!file_exists) {
            data_->counter = 0;
            data_->last_target = -1;
            data_->history_size = 0;
            data_->b_size = 0;
            data_->c_size = 0;
            data_->d_size = 0;
            data_->e_size = 0;
            data_->last_even_id = 0;
            data_->last_odd_id = 0;
            msync(data_, sizeof(SharedData), MS_SYNC);
        }
        
        std::cout << "NodeD: Shared memory initialized. Current counter: " << data_->counter << std::endl;
    }

public:
    SharedMemory(const std::string& user_id) {
        filename_ = user_id + "_shared_data.bin";
        initialize();
    }

    ~SharedMemory() {
        if (data_ != MAP_FAILED) {
            munmap(data_, size_);
        }
        if (fd_ != -1) {
            close(fd_);
        }
    }

    void incrementCounter() {
        data_->counter++;
        msync(data_, sizeof(SharedData), MS_SYNC);
    }

    int getCounter() const {
        return data_->counter;
    }

    void updateMessageHistory(int message_id) {
        if (data_->history_size < 100) {
            data_->message_history[data_->history_size++] = message_id;
        } else {
            // Shift elements left by one
            for (int i = 0; i < 99; i++) {
                data_->message_history[i] = data_->message_history[i + 1];
            }
            data_->message_history[99] = message_id;
        }
        msync(data_, sizeof(SharedData), MS_SYNC);
    }

    void addMessageToNode(int message_id, int node) {
        switch (node) {
            case 0: // Node B
                if (data_->b_size < 100) {
                    data_->messages_to_b[data_->b_size++] = message_id;
                }
                break;
            case 1: // Node C
                if (data_->c_size < 100) {
                    data_->messages_to_c[data_->c_size++] = message_id;
                    data_->last_odd_id = message_id;
                }
                break;
            case 2: // Node D
                if (data_->d_size < 100) {
                    data_->messages_to_d[data_->d_size++] = message_id;
                    data_->last_even_id = message_id;
                }
                break;
            case 3: // Node E
                if (data_->e_size < 100) {
                    data_->messages_to_e[data_->e_size++] = message_id;
                }
                break;
        }
        msync(data_, sizeof(SharedData), MS_SYNC);
    }

    void setLastTarget(int target) {
        data_->last_target = target;
        msync(data_, sizeof(SharedData), MS_SYNC);
    }

    json toJson() const {
        json j;
        j["counter"] = data_->counter;
        j["last_target"] = data_->last_target;
        j["history_size"] = data_->history_size;
        j["b_size"] = data_->b_size;
        j["c_size"] = data_->c_size;
        j["d_size"] = data_->d_size;
        j["e_size"] = data_->e_size;
        j["last_even_id"] = data_->last_even_id;
        j["last_odd_id"] = data_->last_odd_id;

        j["message_history"] = json::array();
        for (int i = 0; i < data_->history_size; ++i) {
            j["message_history"].push_back(data_->message_history[i]);
        }

        j["messages_to_b"] = json::array();
        for (int i = 0; i < data_->b_size; ++i) {
            j["messages_to_b"].push_back(data_->messages_to_b[i]);
        }

        j["messages_to_c"] = json::array();
        for (int i = 0; i < data_->c_size; ++i) {
            j["messages_to_c"].push_back(data_->messages_to_c[i]);
        }

        j["messages_to_d"] = json::array();
        for (int i = 0; i < data_->d_size; ++i) {
            j["messages_to_d"].push_back(data_->messages_to_d[i]);
        }

        j["messages_to_e"] = json::array();
        for (int i = 0; i < data_->e_size; ++i) {
            j["messages_to_e"].push_back(data_->messages_to_e[i]);
        }

        return j;
    }
};

#endif // SHARED_MEMORY_HPP 