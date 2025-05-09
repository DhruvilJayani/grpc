// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: data.proto

#include "data.pb.h"
#include "data.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace crashrecord {

static const char* CrashRecordService_method_names[] = {
  "/crashrecord.CrashRecordService/CreateCrashRecord",
  "/crashrecord.CrashRecordService/GetCrashRecord",
  "/crashrecord.CrashRecordService/ListCrashRecords",
  "/crashrecord.CrashRecordService/DeleteCrashRecord",
};

std::unique_ptr< CrashRecordService::Stub> CrashRecordService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< CrashRecordService::Stub> stub(new CrashRecordService::Stub(channel, options));
  return stub;
}

CrashRecordService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_CreateCrashRecord_(CrashRecordService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetCrashRecord_(CrashRecordService_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_ListCrashRecords_(CrashRecordService_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_DeleteCrashRecord_(CrashRecordService_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status CrashRecordService::Stub::CreateCrashRecord(::grpc::ClientContext* context, const ::crashrecord::CreateCrashRecordRequest& request, ::crashrecord::CreateCrashRecordResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::crashrecord::CreateCrashRecordRequest, ::crashrecord::CreateCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_CreateCrashRecord_, context, request, response);
}

void CrashRecordService::Stub::async::CreateCrashRecord(::grpc::ClientContext* context, const ::crashrecord::CreateCrashRecordRequest* request, ::crashrecord::CreateCrashRecordResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::crashrecord::CreateCrashRecordRequest, ::crashrecord::CreateCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CreateCrashRecord_, context, request, response, std::move(f));
}

void CrashRecordService::Stub::async::CreateCrashRecord(::grpc::ClientContext* context, const ::crashrecord::CreateCrashRecordRequest* request, ::crashrecord::CreateCrashRecordResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_CreateCrashRecord_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::CreateCrashRecordResponse>* CrashRecordService::Stub::PrepareAsyncCreateCrashRecordRaw(::grpc::ClientContext* context, const ::crashrecord::CreateCrashRecordRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::crashrecord::CreateCrashRecordResponse, ::crashrecord::CreateCrashRecordRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_CreateCrashRecord_, context, request);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::CreateCrashRecordResponse>* CrashRecordService::Stub::AsyncCreateCrashRecordRaw(::grpc::ClientContext* context, const ::crashrecord::CreateCrashRecordRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncCreateCrashRecordRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status CrashRecordService::Stub::GetCrashRecord(::grpc::ClientContext* context, const ::crashrecord::GetCrashRecordRequest& request, ::crashrecord::GetCrashRecordResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::crashrecord::GetCrashRecordRequest, ::crashrecord::GetCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetCrashRecord_, context, request, response);
}

void CrashRecordService::Stub::async::GetCrashRecord(::grpc::ClientContext* context, const ::crashrecord::GetCrashRecordRequest* request, ::crashrecord::GetCrashRecordResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::crashrecord::GetCrashRecordRequest, ::crashrecord::GetCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCrashRecord_, context, request, response, std::move(f));
}

void CrashRecordService::Stub::async::GetCrashRecord(::grpc::ClientContext* context, const ::crashrecord::GetCrashRecordRequest* request, ::crashrecord::GetCrashRecordResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCrashRecord_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::GetCrashRecordResponse>* CrashRecordService::Stub::PrepareAsyncGetCrashRecordRaw(::grpc::ClientContext* context, const ::crashrecord::GetCrashRecordRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::crashrecord::GetCrashRecordResponse, ::crashrecord::GetCrashRecordRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetCrashRecord_, context, request);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::GetCrashRecordResponse>* CrashRecordService::Stub::AsyncGetCrashRecordRaw(::grpc::ClientContext* context, const ::crashrecord::GetCrashRecordRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetCrashRecordRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status CrashRecordService::Stub::ListCrashRecords(::grpc::ClientContext* context, const ::crashrecord::ListCrashRecordsRequest& request, ::crashrecord::ListCrashRecordsResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::crashrecord::ListCrashRecordsRequest, ::crashrecord::ListCrashRecordsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_ListCrashRecords_, context, request, response);
}

void CrashRecordService::Stub::async::ListCrashRecords(::grpc::ClientContext* context, const ::crashrecord::ListCrashRecordsRequest* request, ::crashrecord::ListCrashRecordsResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::crashrecord::ListCrashRecordsRequest, ::crashrecord::ListCrashRecordsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_ListCrashRecords_, context, request, response, std::move(f));
}

void CrashRecordService::Stub::async::ListCrashRecords(::grpc::ClientContext* context, const ::crashrecord::ListCrashRecordsRequest* request, ::crashrecord::ListCrashRecordsResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_ListCrashRecords_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::ListCrashRecordsResponse>* CrashRecordService::Stub::PrepareAsyncListCrashRecordsRaw(::grpc::ClientContext* context, const ::crashrecord::ListCrashRecordsRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::crashrecord::ListCrashRecordsResponse, ::crashrecord::ListCrashRecordsRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_ListCrashRecords_, context, request);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::ListCrashRecordsResponse>* CrashRecordService::Stub::AsyncListCrashRecordsRaw(::grpc::ClientContext* context, const ::crashrecord::ListCrashRecordsRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncListCrashRecordsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status CrashRecordService::Stub::DeleteCrashRecord(::grpc::ClientContext* context, const ::crashrecord::DeleteCrashRecordRequest& request, ::crashrecord::DeleteCrashRecordResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::crashrecord::DeleteCrashRecordRequest, ::crashrecord::DeleteCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_DeleteCrashRecord_, context, request, response);
}

void CrashRecordService::Stub::async::DeleteCrashRecord(::grpc::ClientContext* context, const ::crashrecord::DeleteCrashRecordRequest* request, ::crashrecord::DeleteCrashRecordResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::crashrecord::DeleteCrashRecordRequest, ::crashrecord::DeleteCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_DeleteCrashRecord_, context, request, response, std::move(f));
}

void CrashRecordService::Stub::async::DeleteCrashRecord(::grpc::ClientContext* context, const ::crashrecord::DeleteCrashRecordRequest* request, ::crashrecord::DeleteCrashRecordResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_DeleteCrashRecord_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::DeleteCrashRecordResponse>* CrashRecordService::Stub::PrepareAsyncDeleteCrashRecordRaw(::grpc::ClientContext* context, const ::crashrecord::DeleteCrashRecordRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::crashrecord::DeleteCrashRecordResponse, ::crashrecord::DeleteCrashRecordRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_DeleteCrashRecord_, context, request);
}

::grpc::ClientAsyncResponseReader< ::crashrecord::DeleteCrashRecordResponse>* CrashRecordService::Stub::AsyncDeleteCrashRecordRaw(::grpc::ClientContext* context, const ::crashrecord::DeleteCrashRecordRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncDeleteCrashRecordRaw(context, request, cq);
  result->StartCall();
  return result;
}

CrashRecordService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CrashRecordService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CrashRecordService::Service, ::crashrecord::CreateCrashRecordRequest, ::crashrecord::CreateCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](CrashRecordService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::crashrecord::CreateCrashRecordRequest* req,
             ::crashrecord::CreateCrashRecordResponse* resp) {
               return service->CreateCrashRecord(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CrashRecordService_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CrashRecordService::Service, ::crashrecord::GetCrashRecordRequest, ::crashrecord::GetCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](CrashRecordService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::crashrecord::GetCrashRecordRequest* req,
             ::crashrecord::GetCrashRecordResponse* resp) {
               return service->GetCrashRecord(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CrashRecordService_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CrashRecordService::Service, ::crashrecord::ListCrashRecordsRequest, ::crashrecord::ListCrashRecordsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](CrashRecordService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::crashrecord::ListCrashRecordsRequest* req,
             ::crashrecord::ListCrashRecordsResponse* resp) {
               return service->ListCrashRecords(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CrashRecordService_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CrashRecordService::Service, ::crashrecord::DeleteCrashRecordRequest, ::crashrecord::DeleteCrashRecordResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](CrashRecordService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::crashrecord::DeleteCrashRecordRequest* req,
             ::crashrecord::DeleteCrashRecordResponse* resp) {
               return service->DeleteCrashRecord(ctx, req, resp);
             }, this)));
}

CrashRecordService::Service::~Service() {
}

::grpc::Status CrashRecordService::Service::CreateCrashRecord(::grpc::ServerContext* context, const ::crashrecord::CreateCrashRecordRequest* request, ::crashrecord::CreateCrashRecordResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status CrashRecordService::Service::GetCrashRecord(::grpc::ServerContext* context, const ::crashrecord::GetCrashRecordRequest* request, ::crashrecord::GetCrashRecordResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status CrashRecordService::Service::ListCrashRecords(::grpc::ServerContext* context, const ::crashrecord::ListCrashRecordsRequest* request, ::crashrecord::ListCrashRecordsResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status CrashRecordService::Service::DeleteCrashRecord(::grpc::ServerContext* context, const ::crashrecord::DeleteCrashRecordRequest* request, ::crashrecord::DeleteCrashRecordResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace crashrecord

