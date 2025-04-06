#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <cstdlib>
#include "shared_memory.hpp"
#include <openssl/sha.h>
#include <cstring>    // For memcpy
#include <vector>
#include <string>
#include <algorithm>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ClientContext;
using grpc::CreateChannel;
using data::DataMessage;
using data::Empty;
using data::DataService;
using json = nlohmann::json;

// Number of columns expected in the local table.
const int NUM_COLS = 16;
// Name of the CSV file used to store rows locally for NodeB.
const std::string LOCAL_TABLE_FILENAME = "nodeB_table.csv";

// Global fallback counter for row indices.
static int localRowCounter = 0;

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
        std::cerr << "NodeB: Error opening local table file for writing." << std::endl;
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
    SharedMemory shared_memory_;
    json config_;

    json load_config() {
        try {
            std::ifstream file("config.json");
            if (!file.is_open()) {
                std::cerr << "NodeB: Failed to open config.json" << std::endl;
                throw std::runtime_error("Failed to open config.json");
            }
            json config;
            file >> config;
            return config;
        } catch (const std::exception& e) {
            std::cerr << "NodeB: Error loading config: " << e.what() << std::endl;
            throw;
        }
    }

public:
    // Constructor: create shared memory using the given user_id.
    DataServiceImpl(const std::string& user_id) : shared_memory_(user_id) {
        try {
            config_ = load_config();
            std::cout << "NodeB: Configuration loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "NodeB: Error in constructor: " << e.what() << std::endl;
            throw;
        }
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        try {
            std::cout << "NodeB: Received data [ID: " << request->id()
                      << ", Size: " << request->payload().size()
                      << " bytes, Time: " << request->timestamp() << "]" << std::endl;
    
            // Process the payload as a CSV row.
            std::string payload_str = request->payload();
            std::vector<std::string> columns = splitRow(payload_str, ',');
            if (columns.size() < NUM_COLS) {
                columns.resize(NUM_COLS, "");
            } else if (columns.size() > NUM_COLS) {
                columns.resize(NUM_COLS);
            }
    
            // Save the row locally.
            appendRowToCSV(columns);
    
            // Extract the first field as the row index.
            int row_index = 0;
            try {
                row_index = std::stoi(columns[0]);
            } catch (const std::exception& ex) {
                std::cerr << "NodeB: Could not convert first field to integer. Using fallback index." << std::endl;
                row_index = localRowCounter;
                localRowCounter++;
            }
    
            // Compute the SHA-256 hash and get modulo value.
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256(reinterpret_cast<const unsigned char*>(payload_str.data()),
                   payload_str.size(), hash);
    
            uint32_t hash_val;
            std::memcpy(&hash_val, hash, sizeof(uint32_t));
    
            // Use modulo 4 to route:
            // 0 -> NodeB (local), 1 -> NodeC, 2 or 3 -> NodeD.
            unsigned int mod_val = hash_val % 4;
            std::cout << "NodeB: Computed hash mod 4 value: " << mod_val << std::endl;
    
            if (mod_val == 0) {
                // Local branch: update shared memory using extracted index.
                shared_memory_.incrementCounter();
                shared_memory_.addMessageToNode(row_index, 0);
                std::cout << "NodeB: Handled locally. Stored row index " << row_index << " in shared memory." << std::endl;
                return Status::OK;
            } else if (mod_val == 1) {
                // Forward to NodeC.
                std::cout << "NodeB: Mod value is 1, forwarding message " << request->id() << " to Node C" << std::endl;
                shared_memory_.setLastTarget(1);
                // Update shared memory with the extracted index.
                shared_memory_.addMessageToNode(row_index, 1);
                std::string target_address = config_["edges"][0]["address"];
                auto channel = grpc::CreateChannel(target_address, grpc::InsecureChannelCredentials());
                auto stub = DataService::NewStub(channel);
                ClientContext client_context;
                Empty response;
                Status status = stub->PushData(&client_context, *request, &response);
                if (!status.ok()) {
                    std::cerr << "NodeB: Failed to forward message " << request->id()
                              << " to Node C: " << status.error_message() << std::endl;
                    return Status(grpc::StatusCode::INTERNAL, "Failed to forward message");
                }
                return status;
            } else {
                // For mod_val 2 or 3, forward to NodeD.
                std::cout << "NodeB: Mod value is " << mod_val
                          << ", forwarding message " << request->id() << " to Node D" << std::endl;
                shared_memory_.setLastTarget(2);
                // Update shared memory with the extracted index.
                shared_memory_.addMessageToNode(row_index, 2);
                std::string target_address = config_["edges"][1]["address"];
                auto channel = grpc::CreateChannel(target_address, grpc::InsecureChannelCredentials());
                auto stub = DataService::NewStub(channel);
                ClientContext client_context;
                Empty response;
                Status status = stub->PushData(&client_context, *request, &response);
                if (!status.ok()) {
                    std::cerr << "NodeB: Failed to forward message " << request->id()
                              << " to Node D: " << status.error_message() << std::endl;
                    return Status(grpc::StatusCode::INTERNAL, "Failed to forward message");
                }
                return status;
            }
        } catch (const std::exception& e) {
            std::cerr << "NodeB: Error processing message: " << e.what() << std::endl;
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
    std::cout << "NodeB: Server started on " << server_address << std::endl;
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
