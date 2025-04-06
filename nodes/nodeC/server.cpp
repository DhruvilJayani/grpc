#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <cstdlib>
#include "shared_memory.hpp"  // Uses dynamic shared memory (see our revised version)
#include <openssl/sha.h>
#include <cstring>    // For memcpy
#include <vector>
#include <string>
#include <algorithm>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using grpc::CreateChannel;
using data::DataMessage;
using data::Empty;
using data::DataService;
using json = nlohmann::json;

// Number of columns expected in the local table.
const int NUM_COLS = 16;
// Name of the CSV file used to store rows locally for NodeC.
const std::string LOCAL_TABLE_FILENAME = "nodeC_table.csv";

// Helper: Split a string by a given delimiter.
std::vector<std::string> splitRow(const std::string& row, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(row);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Helper: Append a row (vector of strings) to the CSV file.
void appendRowToCSV(const std::vector<std::string>& row) {
    std::ofstream outfile;
    outfile.open(LOCAL_TABLE_FILENAME, std::ios::out | std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "NodeC: Error opening local table file for writing." << std::endl;
        return;
    }
    for (size_t i = 0; i < row.size(); i++) {
        outfile << row[i];
        if (i < row.size() - 1)
            outfile << ",";
    }
    outfile << "\n";
    outfile.close();
}

class DataServiceImpl final : public DataService::Service {
private:
    // Dynamic shared memory instance; its filename is determined by the passed user_id.
    SharedMemory shared_memory_;
    json config_;

    // Load configuration from config.json.
    json load_config() {
        std::ifstream file("config.json");
        if (!file.is_open()) {
            std::cerr << "NodeC: Failed to open config.json" << std::endl;
            throw std::runtime_error("Failed to open config.json");
        }
        json config;
        file >> config;
        return config;
    }

public:
    // Constructor: create shared memory using the provided user_id.
    DataServiceImpl(const std::string& user_id) : shared_memory_(user_id) {
        try {
            config_ = load_config();
            std::cout << "NodeC: Configuration loaded successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "NodeC: Error in constructor: " << e.what() << std::endl;
            throw;
        }
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        try {
            std::cout << "NodeC: Received data - ID: " << request->id() 
                      << ", Size: " << request->payload().size() << std::endl;
    
            // Compute SHA‑256 hash of the payload.
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256(reinterpret_cast<const unsigned char*>(request->payload().data()),
                   request->payload().size(), hash);
    
            // Convert the first 4 bytes of the hash to a 32‑bit unsigned integer.
            uint32_t hash_val;
            std::memcpy(&hash_val, hash, sizeof(uint32_t));
    
            // Use modulo 4 so that valid values are 0, 1, 2, or 3.
            // According to our new logic, if mod == 1 then data belongs to NodeC.
            unsigned int mod_val = hash_val % 4;
            std::cout << "NodeC: Computed hash mod 4 value: " << mod_val << std::endl;
    
            if (mod_val == 1) {
                // Data belongs to NodeC: process locally.
                std::cout << "NodeC: Mod value is 1, saving data locally in tabular format." << std::endl;
                std::string payload_str = request->payload();
                std::vector<std::string> columns = splitRow(payload_str, ',');
                if (columns.size() < NUM_COLS) {
                    columns.resize(NUM_COLS, "");
                } else if (columns.size() > NUM_COLS) {
                    columns.resize(NUM_COLS);
                }
                appendRowToCSV(columns);
    
                // Extract the first field as the row index.
                int row_index = 0;
                try {
                    row_index = std::stoi(columns[0]);
                } catch (const std::exception& ex) {
                    std::cerr << "NodeC: Could not convert first field to integer. Aborting." << std::endl;
                    return Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid index in CSV");
                }
    
                // Update shared memory using the extracted index.
                shared_memory_.incrementCounter();
                shared_memory_.addMessageToNode(row_index, 1);
                std::cout << "NodeC: Data saved locally and row index " << row_index
                          << " stored in shared memory." << std::endl;
                return Status::OK;
            } else if (mod_val == 3) {
                // Data belongs to NodeE: forward the message.
                std::cout << "NodeC: Mod value is 3, forwarding message " << request->id()
                          << " to Node E." << std::endl;
                std::string nodeE_address = config_.value("nodeE_address", "0.0.0.0:50055");
                auto channel = CreateChannel(nodeE_address, grpc::InsecureChannelCredentials());
                auto stub = DataService::NewStub(channel);
                ClientContext client_context;
                Empty response;
                Status status = stub->PushData(&client_context, *request, &response);
                if (!status.ok()) {
                    std::cerr << "NodeC: Failed to forward message " << request->id()
                              << " to Node E: " << status.error_message() << std::endl;
                    return Status(grpc::StatusCode::INTERNAL, "Failed to forward message");
                }
                return status;
            } else {
                // For any other mod value, do nothing.
                std::cout << "NodeC: Mod value " << mod_val 
                          << " does not correspond to Node C or Node E. Ignoring message " 
                          << request->id() << std::endl;
                return Status::OK;
            }
        } catch (const std::exception& e) {
            std::cerr << "NodeC: Error processing message: " << e.what() << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Error processing message");
        }
    }
};

void RunServer(const std::string& server_address, const std::string& user_id) {
    DataServiceImpl service(user_id);
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "NodeC: Server started on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_address> <user_id>" << std::endl;
        return 1;
    }
    std::string server_address(argv[1]);
    std::string user_id(argv[2]);
    RunServer(server_address, user_id);
    return 0;
}
