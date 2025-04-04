#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include "data.grpc.pb.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>
#include "shared_memory.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using data::DataService;
using data::DataMessage;
using data::Empty;

class DataServiceImpl final : public DataService::Service {
private:
    int message_count_;  // Counter for total messages received
    SharedMemory shared_memory_;  // Added shared memory

public:
    DataServiceImpl(const std::string& user_id) : message_count_(0), shared_memory_(user_id) {
        std::cout << "NodeE: Server initialized with user ID: " << user_id << std::endl;
    }

    Status PushData(ServerContext* context, const DataMessage* request, Empty* reply) override {
        try {
            message_count_++;  // Increment message counter
            std::cout << "NodeE: Received data ID " << request->id() << " (Message #" << message_count_ << ")\n";
            
            // Update shared memory with received message
            shared_memory_.incrementCounter();
            shared_memory_.updateMessageHistory(request->id());
            
            // Don't add to E's array since it was already added by the forwarding node
            std::cout << "NodeE: Message " << request->id() << " already in shared memory array\n";
            
            return Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "NodeE: Error processing message: " << e.what() << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Error processing message");
        }
    }
};

void RunServer(const std::string& address, const std::string& user_id) {
    std::string server_address(address);
    DataServiceImpl service(user_id);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    
    // Listen on the given address without authentication mechanism
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case, it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "NodeE: Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <address> <user_id>" << std::endl;
        return 1;
    }

    std::string address = argv[1];
    std::string user_id = argv[2];
    RunServer(address, user_id);

    return 0;
} 