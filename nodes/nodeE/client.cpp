#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "data.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using data::DataService;
using data::DataMessage;
using data::Empty;

class DataClient {
public:
    DataClient(std::shared_ptr<Channel> channel)
        : stub_(DataService::NewStub(channel)) {}

    bool PushData(const DataMessage& message) {
        Empty reply;
        ClientContext context;

        Status status = stub_->PushData(&context, message, &reply);

        if (!status.ok()) {
            std::cerr << "NodeE: RPC failed: " << status.error_message() << std::endl;
            return false;
        }
        return true;
    }

private:
    std::unique_ptr<DataService::Stub> stub_;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server_address>" << std::endl;
        return 1;
    }

    std::string server_address = argv[1];
    DataClient client(grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials()));

    // Example usage
    DataMessage message;
    message.set_id("test_id");
    message.set_payload("test_payload");
    message.set_timestamp("2024-03-21T12:00:00Z");

    bool success = client.PushData(message);
    if (success) {
        std::cout << "NodeE: Message sent successfully" << std::endl;
    }

    return 0;
} 