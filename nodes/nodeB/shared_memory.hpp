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

using json = nlohmann::json;

struct SharedData {
    int counter;
    int last_target;      // 0 for Node C, 1 for Node D
    int message_history[100];
    int messages_to_b[100];
    int messages_to_c[100];
    int messages_to_d[100];
    int history_size;
    int b_size;
    int c_size;
    int d_size;
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
        // Create or open the file
        fd_ = open(filename_.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ == -1) {
            throw std::runtime_error("Failed to open file");
        }

        // Set the file size
        size_ = sizeof(SharedData);
        if (ftruncate(fd_, size_) == -1) {
            close(fd_);
            throw std::runtime_error("Failed to set file size");
        }

        // Map the file into memory
        data_ = static_cast<SharedData*>(mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        if (data_ == MAP_FAILED) {
            close(fd_);
            throw std::runtime_error("Failed to map file");
        }

        // Initialize if this is the first time
        if (data_->counter == 0) {
            data_->counter = 0;
            data_->last_target = -1;
            data_->history_size = 0;
            data_->b_size = 0;
            data_->c_size = 0;
            data_->d_size = 0;
            data_->last_even_id = 0;
            data_->last_odd_id = 0;
            msync(data_, sizeof(SharedData), MS_SYNC);
        }
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
        j["last_even_id"] = data_->last_even_id;
        j["last_odd_id"] = data_->last_odd_id;
        
        std::vector<int> history(data_->message_history, data_->message_history + data_->history_size);
        std::vector<int> to_b(data_->messages_to_b, data_->messages_to_b + data_->b_size);
        std::vector<int> to_c(data_->messages_to_c, data_->messages_to_c + data_->c_size);
        std::vector<int> to_d(data_->messages_to_d, data_->messages_to_d + data_->d_size);
        
        j["message_history"] = history;
        j["messages_to_b"] = to_b;
        j["messages_to_c"] = to_c;
        j["messages_to_d"] = to_d;
        return j;
    }
};

#endif // SHARED_MEMORY_HPP 