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
    Status status = stub_->PushData(&context, request, &reply);
    if (!status.ok()) {
      std::cerr << "Error forwarding data: " << status.error_message() << std::endl;
    }
  }

 private:
  std::unique_ptr<DataService::Stub> stub_;
};

class DataServiceImpl final : public DataService::Service {
private:
    SharedMemory shared_memory_;

public:
    DataServiceImpl(const std::string& user_id) : shared_memory_(user_id) {}

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        std::cout << "Received message " << request->id() << " from Node A" << std::endl;
        
        // Increment the counter
        shared_memory_.incrementCounter();
        
        // Update message history
        shared_memory_.updateMessageHistory(request->id());
        
        // Forward to either Node C or Node D
        int target = rand() % 2;  // 0 for Node C, 1 for Node D
        shared_memory_.setLastTarget(target);
        
        std::string target_node = (target == 0) ? "Node C" : "Node D";
        std::cout << "Forwarding message to " << target_node << std::endl;
        
        return Status::OK;
    }
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

void RunServer(const std::string& server_address, const std::string& user_id) {
    DataServiceImpl service(user_id);
    
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
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