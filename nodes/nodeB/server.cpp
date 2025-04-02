#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <cstdlib>

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
    Status status = stub_->PushData(&context, request, &reply);
    if (!status.ok()) {
      std::cerr << "Error forwarding data: " << status.error_message() << std::endl;
    }
  }

 private:
  std::unique_ptr<DataService::Stub> stub_;
};

class DataServiceImpl final : public DataService::Service {
 public:
  DataServiceImpl(const json& config) 
      : config_(config), counter_(0), last_target_(-1) {
    std::cout << "Server initialized with counter = 0" << std::endl;
  }

  Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
    try {
      std::cout << "NodeB: Received data ID " << request->id() << "\n";

      // Update counter
      counter_++;

      // Get last target and determine next target
      int next_target = (last_target_ == 0) ? 1 : 0;  // Toggle between 0 (C) and 1 (D)
      
      const auto& edges = config_["edges"];
      if (edges.size() < 2) {
        std::cerr << "Error: Not enough edges configured" << std::endl;
        return Status(grpc::StatusCode::INTERNAL, "Not enough edges configured");
      }
      
      const auto& target_edge = edges[next_target];
      std::string target = target_edge["ip"].get<std::string>() + ":" + std::to_string(target_edge["port"].get<int>());
      std::string target_name = target_edge["id"].get<std::string>();

      std::cout << "NodeB: Forwarding data ID " << request->id() 
                << " to " << target_name << " at " << target 
                << " (Total messages processed: " << counter_ << ")" << std::endl;

      // Update last target
      last_target_ = next_target;

      // Forward the message
      auto channel = CreateChannel(target, grpc::InsecureChannelCredentials());
      DataServiceClient client(channel);
      client.PushData(*request);

      return Status::OK;
    } catch (const std::exception& e) {
      std::cerr << "Error in PushData: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INTERNAL, "Internal server error");
    }
  }

 private:
  json config_;
  int counter_;
  int last_target_;
};

json load_config() {
    try {
        std::ifstream f("config.json");
        if (!f.is_open()) {
            throw std::runtime_error("Could not open config.json");
        }
        return json::parse(f);
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        throw;
    }
}

int main() {
    try {
        // Load configuration
        json config = load_config();
        std::string server_address("0.0.0.0:50051");
        
        std::cout << "NodeB: Starting server on " << server_address << std::endl;
        std::cout << "NodeB: Configured edges:" << std::endl;
        for (const auto& edge : config["edges"]) {
            std::cout << "  - " << edge["id"] << " at " 
                      << edge["ip"] << ":" << edge["port"] << std::endl;
        }

        DataServiceImpl service(config);
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        
        std::cout << "NodeB: Server started successfully" << std::endl;
        server->Wait();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}