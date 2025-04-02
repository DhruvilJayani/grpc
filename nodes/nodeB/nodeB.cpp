#include <iostream>
#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "shared_memory.hpp"
#include "nodeB.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class NodeBServiceImpl final : public NodeB::Service {
public:
    NodeBServiceImpl(const std::string& user_id) {
        std::cout << "Initializing NodeB with user_id: " << user_id << std::endl;
        try {
            shared_memory_ = std::make_unique<SharedMemoryManager>(user_id);
            std::cout << "Successfully initialized shared memory" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize shared memory: " << e.what() << std::endl;
            throw;
        }
    }

    Status ProcessMessage(ServerContext* context, const Message* request, Response* response) override {
        try {
            std::cout << "Processing message ID: " << request->message_id() << std::endl;
            
            // Increment counter in shared memory
            shared_memory_->incrementCounter();
            
            // Update message history
            shared_memory_->updateMessageHistory(request->message_id());
            
            // Determine target node (C or D) based on message ID
            int target = request->message_id() % 2;  // 0 for Node C, 1 for Node D
            shared_memory_->setLastTarget(target);
            
            // Set response
            response->set_target_node(target == 0 ? "Node C" : "Node D");
            response->set_status("Success");
            
            std::cout << "Successfully processed message. Counter: " << shared_memory_->getCounter() << std::endl;
            return Status::OK;
        } catch (const std::exception& e) {
            std::cerr << "Error processing message: " << e.what() << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Internal error processing message");
        }
    }

private:
    std::unique_ptr<SharedMemoryManager> shared_memory_;
};

void RunServer(const std::string& address, const std::string& user_id) {
    std::string server_address(address);
    std::cout << "Starting NodeB server with address: " << server_address << " and user_id: " << user_id << std::endl;
    
    NodeBServiceImpl service(user_id);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::cout << "NodeB server listening on " << server_address << std::endl;
    std::unique_ptr<Server> server(builder.BuildAndStart());
    server->Wait();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <address> <user_id>" << std::endl;
        return 1;
    }

    try {
        RunServer(argv[1], argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 