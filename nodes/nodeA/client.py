# server.py
import grpc
from concurrent import futures
import time

# Import your generated classes
import data_pb2
import data_pb2_grpc

class CrashRecordServiceServicer(data_pb2_grpc.CrashRecordServiceServicer):
    def StreamCrashRecords(self, request_iterator, context):
        count = 0
        for crash_record in request_iterator:
            # Process each incoming CrashRecord
            print(f"Received record {crash_record.collisionId} at {crash_record.crashDate} {crash_record.crashTime}")
            count += 1
        # Once done, send a response
        return data_pb2.StreamCrashRecordsResponse(
            success=True,
            message=f"Received {count} records successfully."
        )

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    data_pb2_grpc.add_CrashRecordServiceServicer_to_server(CrashRecordServiceServicer(), server)
    server.add_insecure_port('[::]:50051')  # Use your target port
    server.start()
    print("Server started on port 50051.")
    try:
        while True:
            time.sleep(86400)
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    serve()
