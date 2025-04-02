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
        std::ifstream file("config.json");
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open config.json");
        }
        json config;
        file >> config;
        return config;
    }

public:
    DataServiceImpl(const std::string& user_id) : shared_memory_(user_id) {
        try {
            config_ = load_config();
            std::cout << "NodeB: Loaded configuration for Node C and Node D" << std::endl;
            for (const auto& edge : config_["edges"]) {
                std::cout << "NodeB: Configured edge - " << edge["id"] << " at " 
                          << edge["ip"] << ":" << edge["port"] << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading config: " << e.what() << std::endl;
            throw;
        }
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        try {
            std::cout << "\n==========================================" << std::endl;
            std::cout << "NodeB: PushData method called!" << std::endl;
            std::cout << "NodeB: Received message ID " << request->id() << " from Node A" << std::endl;
            std::cout << "NodeB: Message payload: " << request->payload() << std::endl;
            std::cout << "NodeB: Message timestamp: " << request->timestamp() << std::endl;
            std::cout << "==========================================\n" << std::endl;
            
            // Increment the counter
            shared_memory_.incrementCounter();
            std::cout << "NodeB: Counter incremented to " << shared_memory_.getCounter() << std::endl;
            
            // Update message history
            shared_memory_.updateMessageHistory(request->id());
            std::cout << "NodeB: Message history updated" << std::endl;
            
            // Forward to either Node C or Node D
            int target = rand() % 2;  // 0 for Node C, 1 for Node D
            shared_memory_.setLastTarget(target);
            
            const auto& edges = config_["edges"];
            if (edges.size() < 2) {
                std::cerr << "Error: Not enough edges configured" << std::endl;
                return Status(grpc::StatusCode::INTERNAL, "Not enough edges configured");
            }
            
            const auto& target_edge = edges[target];
            std::string target_address = target_edge["ip"].get<std::string>() + ":" + 
                                       std::to_string(target_edge["port"].get<int>());
            std::string target_name = target_edge["id"].get<std::string>();

            std::cout << "NodeB: Forwarding message ID " << request->id() 
                      << " to " << target_name << " at " << target_address 
                      << " (Total messages processed: " << shared_memory_.getCounter() << ")" << std::endl;

            // Create channel to target node
            std::cout << "NodeB: Creating channel to " << target_address << "..." << std::endl;
            auto channel = CreateChannel(target_address, grpc::InsecureChannelCredentials());
            DataServiceClient client(channel);
            client.PushData(*request);

            return Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "Error in PushData: " << e.what() << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Internal server error");
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