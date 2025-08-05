from .protocol import RequestType, ResponseType
from .config import SERVER_IP, SERVER_PORT
import struct
import socket

class CommunicationManager:

    def __init__(self):
        self.fmt = 'I256s'
        self.pack_size = struct.calcsize(self.fmt)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect()

    """ Connect to the game server """
    def connect(self):
        print(f"Connecting to server at {SERVER_IP}:{SERVER_PORT}...")
        self.sock.connect((SERVER_IP, SERVER_PORT))
        message, status = self.__receive_response()
        if status == ResponseType.ERROR:
            print(f"Connection failed: {message}")
            exit(1)
        print(f"Connected!")
        self.__login()

    """ Send data to the server (public method for general use) """
    def send_data(self, data):
        if isinstance(data, str):
            data = data.encode('utf-8')
        elif isinstance(data, int):
            data = data.to_bytes(4, 'big')
        try:
            self.sock.sendall(data)
        except socket.error as e:
            print(f"Failed to send data: {e}")

    """ Receive data from the server (public method for general use) """
    def receive_data(self, buffer_size=1024) -> bytes | None:
        try:
            data = self.sock.recv(buffer_size)
            return data
        except socket.error as e:
            print(f"Failed to receive data: {e}")
            return None

    """ Send a request to the server and receive a response """
    def send_request(self, request_type: RequestType):
        self.send_data(request_type.value)
        print("Request sent to the server.")
        return self.__receive_response()

    """ Login method to handle user login """
    def __login(self):
        while True:
            username = input("Enter username: ")
            if len(username) <= 16:
                username = username.encode('utf-8')
                self.send_data(username)
                message, status = self.__receive_response()
                if status == ResponseType.OK:
                    print(f"Login successful: {message}")
                    break
                else:
                    print(f"Login failed: {message}")

    """ Receive a response from the server, ensuring the data is complete """
    def __receive_response(self):
        try:
            data = self.receive_data(self.pack_size)
            if data is None:
                print("No data received")
                raise ConnectionError("No data received from the server.")
            if len(data) < self.pack_size:
                raise ValueError(f"Received {len(data)} bytes while expecting {self.pack_size} bytes: data is incomplete or malformed.")
            status_code, message = struct.unpack(self.fmt, data)
            message = message.decode('utf-8').rstrip('\0')
            return message, ResponseType(socket.ntohl(status_code))
        except Exception as e:
            print(f"Error receiving response: {e}")
            exit(1)
