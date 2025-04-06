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
// Name of the CSV file used to store rows locally for NodeD.
const std::string LOCAL_TABLE_FILENAME = "nodeD_table.csv";

// Global counter for the number of rows saved locally (for NodeD).
// (This may still be used for logging purposes if desired.)
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
    // Open in append mode.
    outfile.open(LOCAL_TABLE_FILENAME, std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "NodeD: Error opening local table file for writing." << std::endl;
        return;
    }
    // Write the row: join columns with commas.
    for (size_t i = 0; i < row.size(); i++) {
        outfile << row[i];
        if (i < row.size() - 1)
            outfile << ",";
    }
    outfile << "\n";
    outfile.close();
}

// Client class for forwarding messages to Node E.
class DataServiceClient {
 public:
  DataServiceClient(std::shared_ptr<Channel> channel)
      : stub_(DataService::NewStub(channel)) {}

  void PushData(const DataMessage& request) {
    Empty reply;
    ClientContext context;
    std::cout << "NodeD: Attempting to forward message ID " << request.id() 
              << " to Node E..." << std::endl;
    Status status = stub_->PushData(&context, request, &reply);
    if (!status.ok()) {
      std::cerr << "NodeD: Error forwarding data: " << status.error_message() << std::endl;
    } else {
      std::cout << "NodeD: Successfully forwarded message ID " << request.id() 
                << " to Node E" << std::endl;
    }
  }

 private:
  std::unique_ptr<DataService::Stub> stub_;
};

class DataServiceImpl final : public DataService::Service {
private:
    // Dynamic shared memory instance; its filename is determined by the passed user_id.
    SharedMemory shared_memory_;
    json config_;

    // Load configuration from config.json.
    json load_config() {
        std::ifstream file("config.json");
        if (!file.is_open()) {
            std::cerr << "NodeD: Failed to open config.json" << std::endl;
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
            std::cout << "NodeD: Configuration loaded successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "NodeD: Error in constructor: " << e.what() << std::endl;
            throw;
        }
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        try {
            std::cout << "NodeD: Received data - ID: " << request->id() 
                      << ", Size: " << request->payload().size() << std::endl;
            
            // Compute SHA‑256 hash of the payload.
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256(reinterpret_cast<const unsigned char*>(request->payload().data()),
                   request->payload().size(), hash);
            
            // Convert the first 4 bytes of the hash to a 32‑bit unsigned integer.
            uint32_t hash_val;
            std::memcpy(&hash_val, hash, sizeof(uint32_t));
            
            // Compute modulo 4 so that valid results are 0, 1, 2, or 3.
            unsigned int mod_val = hash_val % 4;
            std::cout << "NodeD: Computed hash mod 4 value: " << mod_val << std::endl;
            
            if (mod_val == 2) {
                // Data belongs to NodeD: save the row locally and update shared memory.
                std::cout << "NodeD: Mod value is 2, saving data locally in tabular format." << std::endl;
                
                // Convert payload to string.
                std::string payload_str = request->payload();
                // Split the row using comma as delimiter.
                std::vector<std::string> columns = splitRow(payload_str, ',');
                // Ensure there are exactly NUM_COLS columns.
                if (columns.size() < NUM_COLS) {
                    columns.resize(NUM_COLS, "");
                } else if (columns.size() > NUM_COLS) {
                    columns.resize(NUM_COLS);
                }
                // Append the row to the local CSV file.
                appendRowToCSV(columns);
                
                // **Extract the first value as the index.**
                int row_index = 0;
                try {
                    row_index = std::stoi(columns[0]);
                } catch (const std::exception& ex) {
                    std::cerr << "NodeD: Error converting first column to integer, defaulting to local counter." << std::endl;
                    row_index = localRowCounter;
                    localRowCounter++;
                }
                
                // Update shared memory: increment counter and add the extracted row index for NodeD.
                shared_memory_.incrementCounter();
                // For NodeD, we use node value 2.
                shared_memory_.addMessageToNode(row_index, 2);
                return Status::OK;
            } else if (mod_val == 3) {
                // Data belongs to NodeE: forward the message.
                std::cout << "NodeD: Mod value is 3, forwarding message " << request->id() 
                          << " to Node E." << std::endl;
                std::string nodeE_address = config_.value("nodeE_address", "0.0.0.0:50055");
                auto channel = CreateChannel(nodeE_address, grpc::InsecureChannelCredentials());
                auto stub = DataService::NewStub(channel);
                ClientContext client_context;
                Empty response;
                Status status = stub->PushData(&client_context, *request, &response);
                if (!status.ok()) {
                    std::cerr << "NodeD: Failed to forward message " << request->id() 
                              << " to Node E: " << status.error_message() << std::endl;
                    return Status(grpc::StatusCode::INTERNAL, "Failed to forward message");
                }
                return status;
            } else {
                // For any other mod value, ignore the message.
                std::cout << "NodeD: Mod value " << mod_val 
                          << " does not correspond to Node D or Node E. Ignoring message " 
                          << request->id() << std::endl;
                return Status::OK;
            }
        } catch (const std::exception& e) {
            std::cerr << "NodeD: Error processing message: " << e.what() << std::endl;
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
    std::cout << "NodeD: Server started on " << server_address << std::endl;
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
