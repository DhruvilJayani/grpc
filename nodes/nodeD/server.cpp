#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using data::DataMessage;
using data::Empty;
using data::DataService;
using json = nlohmann::json;

class DataServiceImpl final : public DataService::Service {
  Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
    std::cout << "NodeD: Received data ID " << request->id() << "\n";
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
        std::string server_address("0.0.0.0:50053");
        
        std::cout << "NodeD: Starting server on " << server_address << std::endl;

        DataServiceImpl service;
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        
        std::cout << "NodeD: Server started successfully" << std::endl;
        server->Wait();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 