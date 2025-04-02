#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <user_id>" << std::endl;
        return 1;
    }

    std::string user_id = argv[1];
    std::string filename = user_id + "_data.json";
    
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            std::cerr << "Is the process running?" << std::endl;
            return 1;
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errs;
        if (!Json::parseFromStream(builder, file, &root, &errs)) {
            std::cerr << "Failed to parse JSON: " << errs << std::endl;
            return 1;
        }

        std::cout << "\n=== Shared Data Contents ===" << std::endl;
        std::cout << "Total messages processed: " << root["counter"].asInt() << std::endl;
        std::cout << "Last target node: " << (root["last_target"].asInt() == -1 ? "None" : 
                                            (root["last_target"].asInt() == 0 ? "Node C" : "Node D")) << std::endl;
        
        std::cout << "\nMessage History:" << std::endl;
        const Json::Value& history = root["message_history"];
        if (history.empty()) {
            std::cout << "No messages in history" << std::endl;
        } else {
            for (Json::Value::ArrayIndex i = 0; i < history.size(); i++) {
                std::cout << "Message " << i + 1 << ": " << history[i].asInt() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading data: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 