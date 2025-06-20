import socket
from .config import SERVER_IP, SERVER_PORT
from .communication import CommunicationManager
from .protocol import ResponseType

class NetworkManager:

    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    """ Connect to the game server and handle the login process """
    def connect(self, communication: CommunicationManager):
        try:
            self.sock.connect((SERVER_IP, SERVER_PORT))
            message, status = communication.receive_response(self)
            if status == ResponseType.ERROR:
                print(f"Connection failed: {message}")
                self.sock.close()
                exit(1)
            print(f"Connected to server at {SERVER_IP}:{SERVER_PORT}")
            self.login(communication)
        except socket.error as e:
            print(f"Failed to connect to server: {e}")
            print("Please check the server is up and running or address and port in config.py.")
            exit(1)

    """ Send raw binary data to the server """
    def send(self, data: bytes):
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
        
    """ Login method to handle user login """
    def login(self, communication: CommunicationManager):
        while True:
            username = input("Enter username: ")
            if len(username) <= 16:
                message = username.encode('utf-8')
                self.send(message)
                message, status = communication.receive_response(self)
                if status == ResponseType.OK:
                    print(f"Login successful: {message}")
                    break
                else:
                    print(f"Login failed: {message}")

    