#pragma once

#include <string>
#include <fstream>
#include <json/json.h>
#include <cstring>
#include <iostream>

class SharedMemory {
private:
    std::string filename_;
    Json::Value data_;

public:
    SharedMemory(const std::string& user_id) {
        filename_ = user_id + "_data.json";
        initialize();
    }

    ~SharedMemory() {
        // Save data before destruction
        saveData();
    }

    void initialize() {
        // Try to load existing data
        std::ifstream file(filename_);
        if (file.is_open()) {
            Json::CharReaderBuilder builder;
            std::string errs;
            if (!Json::parseFromStream(builder, file, &data_, &errs)) {
                // If parsing fails, initialize with default values
                initializeDefaultValues();
            }
            file.close();
        } else {
            // If file doesn't exist, initialize with default values
            initializeDefaultValues();
            saveData();
        }
    }

    void initializeDefaultValues() {
        data_["counter"] = 0;
        data_["last_target"] = -1;
        data_["message_history"] = Json::Value(Json::arrayValue);
    }

    void saveData() {
        std::ofstream file(filename_);
        if (file.is_open()) {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = ""; // Make it compact
            std::string jsonString = Json::writeString(builder, data_);
            file << jsonString;
            file.close();
        }
    }

    void incrementCounter() {
        data_["counter"] = data_["counter"].asInt() + 1;
        saveData();
    }

    void updateMessageHistory(int message) {
        Json::Value& history = data_["message_history"];
        
        // Keep only the last 10 messages
        if (history.size() >= 10) {
            // Remove the oldest message
            for (Json::ArrayIndex i = 0; i < history.size() - 1; i++) {
                history[i] = history[i + 1];
            }
            history.resize(9);
        }
        
        // Add the new message
        history.append(message);
        saveData();
    }

    void setLastTarget(int target) {
        data_["last_target"] = target;
        saveData();
    }

    int getCounter() const {
        return data_["counter"].asInt();
    }

    int getLastTarget() const {
        return data_["last_target"].asInt();
    }
}; 