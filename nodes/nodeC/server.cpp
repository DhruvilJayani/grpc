#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>

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

public:
    DataServiceImpl(const std::string& nodeE_address) : message_count_(0) {
        // Create a channel to Node E using the provided address
        auto channel = grpc::CreateChannel(nodeE_address, grpc::InsecureChannelCredentials());
        nodeE_stub_ = DataService::NewStub(channel);
        std::cout << "NodeC: Connected to Node E at " << nodeE_address << std::endl;
        std::cout << "NodeC: Will distribute messages evenly (C gets extra on odd counts)" << std::endl;
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        message_count_++;  // Increment message counter
        std::cout << "NodeC: Received data ID " << request->id() << " (Message #" << message_count_ << ")\n";
        
        // If message_count is odd, keep it at C
        // If message_count is even, forward it to E
        bool should_forward = (message_count_ % 2 == 0);
        
        if (should_forward) {
            std::cout << "NodeC: Forwarding data ID " << request->id() << " to Node E (even count)\n";
            ClientContext client_context;
            Empty response;
            Status status = nodeE_stub_->PushData(&client_context, *request, &response);
            
            if (status.ok()) {
                std::cout << "NodeC: Successfully forwarded data ID " << request->id() << " to Node E\n";
            } else {
                std::cerr << "NodeC: Failed to forward data ID " << request->id() << " to Node E: " 
                          << status.error_message() << "\n";
            }
        } else {
            std::cout << "NodeC: Keeping data ID " << request->id() << " locally (odd count)\n";
        }
        
        // Print current distribution
        int c_count = (message_count_ + 1) / 2;  // Node C gets (n+1)/2 messages
        int e_count = message_count_ / 2;         // Node E gets n/2 messages
        std::cout << "Current distribution - Node C: " << c_count << ", Node E: " << e_count << std::endl;
        
        return Status::OK;
    }
};

json load_config() {
    std::ifstream f("config.json");
    return json::parse(f);
}

int main() {
    try {
        // Load configuration
        json config = load_config();
        std::string server_address("0.0.0.0:50052");
        std::string nodeE_address = config.value("nodeE_address", "0.0.0.0:50055");
        
        std::cout << "NodeC: Starting server on " << server_address << std::endl;

        DataServiceImpl service(nodeE_address);
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        
        std::cout << "NodeC: Server started successfully" << std::endl;
        server->Wait();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 