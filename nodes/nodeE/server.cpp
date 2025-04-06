#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include "data.grpc.pb.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>
#include "shared_memory.hpp"
#include <vector>
#include <algorithm>
#include <cstring>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using data::DataService;
using data::DataMessage;
using data::Empty;
using json = nlohmann::json;

// Number of columns expected in the local table.
const int NUM_COLS = 16;
// Name of the CSV file used to store rows locally for NodeE.
const std::string LOCAL_TABLE_FILENAME = "nodeE_table.csv";

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
    outfile.open(LOCAL_TABLE_FILENAME, std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "NodeE: Error opening local table file for writing." << std::endl;
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
    int message_count_;  // Local counter for messages received.
    SharedMemory shared_memory_;  // Dynamic shared memory instance.

public:
    DataServiceImpl(const std::string& user_id)
        : message_count_(0), shared_memory_(user_id) {
        std::cout << "NodeE: Server initialized with user ID: " << user_id << std::endl;
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        try {
            message_count_++;  // Increment local message counter.
            std::cout << "NodeE: Received data ID " << request->id()
                      << " (Message #" << message_count_ << ")\n";
            
            // Save data locally:
            std::string payload_str = request->payload();
            std::vector<std::string> columns = splitRow(payload_str, ',');
            // Ensure there are exactly NUM_COLS columns.
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
                std::cerr << "NodeE: Failed to convert first field to integer. Using 0 as index. Error: " << ex.what() << std::endl;
            }
            
            // Update dynamic shared memory:
            shared_memory_.incrementCounter();
            // For NodeE, we use node value 3.
            shared_memory_.addMessageToNode(row_index, 3);
            
            std::cout << "NodeE: Data saved locally and row index " << row_index
                      << " stored in shared memory.\n";
            return Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "NodeE: Error processing message: " << e.what() << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Error processing message");
        }
    }
};

void RunServer(const std::string& address, const std::string& user_id) {
    DataServiceImpl service(user_id);
    
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    
    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "NodeE: Server listening on " << address << std::endl;
    
    server->Wait();
}

int main(int argc, char** argv) {
    if(argc != 3) {
         std::cerr << "Usage: " << argv[0] << " <address> <user_id>" << std::endl;
         return 1;
    }
    std::string address = argv[1];
    std::string user_id = argv[2];
    RunServer(address, user_id);
    return 0;
}
