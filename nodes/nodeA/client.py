import grpc
import time
import json
import data_pb2
import data_pb2_grpc

def load_config():
    with open('config.json', 'r') as f:
        return json.load(f)

def generate_rows(num_rows):
    """
    Generate `num_rows` rows with 16 comma-separated columns.
    The first column is the row index (starting from 0), and the next 15 columns
    are generated as A{row}, B{row}, ..., O{row}.
    """
    rows = []
    letters = "ABCDEFGHIJKLMNO"  # 15 letters
    for i in range(num_rows):
        # Start with the index as the first field.
        fields = [str(i)]
        # Append 15 fields based on letters and the row index.
        for letter in letters:
            fields.append(f"{letter}{i}")
        # Join fields with commas.
        row = ",".join(fields)
        rows.append(row)
    return rows

def send_data(stub, id, row_data):
    data = data_pb2.DataMessage(
        id=id,
        payload=row_data.encode('utf-8'),
        timestamp=str(time.time())
    )
    stub.PushData(data)
    print(f"Sent data ID: {id} with payload: {row_data}")

def run():
    # Load configuration.
    config = load_config()
    target = config['target']
    server_address = f"{target['ip']}:{target['port']}"
    
    print(f"Connecting to server at {server_address}")
    channel = grpc.insecure_channel(server_address)
    stub = data_pb2_grpc.DataServiceStub(channel)
    
    # Set the number of rows to generate.
    num_rows = 50  # Change to 100, 1000, etc. as needed.
    
    # Generate rows dynamically.
    rows = generate_rows(num_rows)
    
    # Send each row as a separate message.
    for i, row in enumerate(rows, start=1):
        send_data(stub, i, row)
        time.sleep(1)  # 1-second delay between sends

if __name__ == '__main__':
    run()
