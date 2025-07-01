import socket
from .config import SERVER_IP, SERVER_PORT
from .protocol import RequestType

class NetworkManager:

    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    """ Connect to the game server """
    def connect(self):
        print(f"Connecting to server at {SERVER_IP}:{SERVER_PORT}...")
        self.sock.connect((SERVER_IP, SERVER_PORT))

    """ Send data to the server """
    def send(self, data):
        if isinstance(data, str):
            data = data.encode('utf-8')
        elif isinstance(data, int):
            data = data.to_bytes(4, 'big')
        try:
            self.sock.sendall(data)
        except socket.error as e:
            print(f"Failed to send data: {e}")

    """ Receive data from the server with a specified buffer size """
    def receive(self, buffer_size=1024) -> bytes | None:
        try:
            data = self.sock.recv(buffer_size)
            """ if not data:
                print("Connection closed by the server.")
                return None """
            return data
        except socket.error as e:
            print(f"Failed to receive data: {e}")
            return None

    