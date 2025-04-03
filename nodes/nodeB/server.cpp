#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <cstdlib>
#include "shared_memory.hpp"

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

// Client class for forwarding data
class DataServiceClient {
 public:
  DataServiceClient(std::shared_ptr<Channel> channel)
      : stub_(DataService::NewStub(channel)) {}

  void PushData(const DataMessage& request) {
    Empty reply;
    ClientContext context;
    std::cout << "NodeB: Attempting to forward message ID " << request.id() << "..." << std::endl;
    Status status = stub_->PushData(&context, request, &reply);
    if (!status.ok()) {
      std::cerr << "NodeB: Error forwarding data: " << status.error_message() << std::endl;
    } else {
      std::cout << "NodeB: Successfully forwarded message ID " << request.id() << std::endl;
    }
  }

 private:
  std::unique_ptr<DataService::Stub> stub_;
};

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
            // Log received data
            std::cout << "NodeB: Received data [ID: " << request->id() 
                      << ", Size: " << request->payload().size() 
                      << " bytes, Time: " << request->timestamp() << "]" << std::endl;

            // Update shared memory
            shared_memory_.incrementCounter();
            shared_memory_.updateMessageHistory(request->id());

            // Determine target node based on message ID
            int target_node = (request->id() % 2 == 0) ? 1 : 0; // Even IDs to D (1), odd IDs to C (0)
            
            // Update last target in shared memory
            shared_memory_.setLastTarget(target_node);
            
            // Add message to appropriate node's history
            shared_memory_.addMessageToNode(request->id(), target_node + 1); // +1 because 0 is Node B

            // Get target address from config
            if (!config_.contains("edges") || config_["edges"].size() < 2) {
                std::cerr << "NodeB: Invalid configuration - missing edges" << std::endl;
                return Status(grpc::StatusCode::INTERNAL, "Invalid configuration");
            }

            std::string target_address;
            if (target_node == 0) {
                target_address = config_["edges"][0]["address"];
                std::cout << "NodeB: Forwarding message " << request->id() << " to Node C" << std::endl;
            } else {
                target_address = config_["edges"][1]["address"];
                std::cout << "NodeB: Forwarding message " << request->id() << " to Node D" << std::endl;
            }

            // Create channel to target node
            auto channel = grpc::CreateChannel(target_address, grpc::InsecureChannelCredentials());
            auto stub = DataService::NewStub(channel);

            // Forward the message
            ClientContext client_context;
            Empty response;
            Status status = stub->PushData(&client_context, *request, &response);

            if (!status.ok()) {
                std::cerr << "NodeB: Failed to forward message " << request->id() 
                          << " to target node: " << status.error_message() << std::endl;
                return Status(grpc::StatusCode::INTERNAL, "Failed to forward message");
            }

            std::cout << "NodeB: Successfully forwarded message " << request->id() << std::endl;
            return Status::OK;
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