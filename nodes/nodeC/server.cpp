#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>
#include "shared_memory.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using data::DataMessage;
using data::Empty;
using data::DataService;
using json = nlohmann::json;

class DataServiceImpl final : public DataService::Service {
private:
    std::unique_ptr<DataService::Stub> nodeE_stub_;
    int message_count_;  // Counter for total messages received
    SharedMemory shared_memory_;  // Added shared memory

public:
    DataServiceImpl(const std::string& nodeE_address, const std::string& user_id) 
        : message_count_(0), shared_memory_(user_id) {
        // Create a channel to Node E using the provided address
        auto channel = grpc::CreateChannel(nodeE_address, grpc::InsecureChannelCredentials());
        nodeE_stub_ = DataService::NewStub(channel);
        std::cout << "NodeC: Connected to Node E at " << nodeE_address << std::endl;
        std::cout << "NodeC: Will distribute messages evenly (C gets extra on odd counts)" << std::endl;
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        try {
            // Increment message counter and log received data
            shared_memory_.incrementCounter();
            int message_count = shared_memory_.getCounter();
            std::cout << "NodeC: Received data - ID: " << request->id() 
                      << ", Message: " << request->message() 
                      << ", Count: " << message_count << std::endl;

            // Update shared memory - only add to history once
            shared_memory_.updateMessageHistory(request->id());
            
            // Forward to Node E if count is even
            if (message_count % 2 == 0) {
                std::cout << "NodeC: Forwarding message " << request->id() << " to Node E" << std::endl;
                try {
                    // Forward to Node E
                    Empty response;
                    Status status = nodeE_stub_->PushData(context, request, &response);
                    if (!status.ok()) {
                        std::cerr << "NodeC: Failed to forward to Node E: " << status.error_message() << std::endl;
                        throw std::runtime_error("Failed to forward to Node E");
                    }
                    // Only add to E's array in shared memory
                    shared_memory_.addMessageToNode(request->id(), 3); // 3 is Node E
                } catch (const std::exception& e) {
                    std::cerr << "NodeC: Exception while forwarding to Node E: " << e.what() << std::endl;
                    throw;
                }
            } else {
                // Keep message locally if count is odd
                std::cout << "NodeC: Keeping message " << request->id() << " locally" << std::endl;
                // Add to C's array in shared memory
                shared_memory_.addMessageToNode(request->id(), 1); // 1 is Node C
            }

            // Print current distribution
            std::cout << "NodeC: Current distribution - C: " << shared_memory_.getCounter() / 2 + 1 
                      << ", E: " << shared_memory_.getCounter() / 2 << std::endl;

            return Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "NodeC: Error in PushData: " << e.what() << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Error processing message");
        }
    }
};

json load_config() {
    std::ifstream f("config.json");
    return json::parse(f);
}

void RunServer(const std::string& server_address, const std::string& user_id) {
    try {
        // Load configuration
        json config = load_config();
        std::string nodeE_address = config.value("nodeE_address", "0.0.0.0:50055");
        
        std::cout << "NodeC: Starting server on " << server_address << std::endl;

        DataServiceImpl service(nodeE_address, user_id);
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        
        std::cout << "NodeC: Server started successfully" << std::endl;
        server->Wait();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <user_id>" << std::endl;
        return 1;
    }
    
    std::string server_address("0.0.0.0:50052");
    std::string user_id(argv[1]);
    
    RunServer(server_address, user_id);
    
    return 0;
} 