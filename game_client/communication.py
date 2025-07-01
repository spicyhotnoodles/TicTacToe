from .protocol import RequestType, ResponseType
from .network import NetworkManager
from .ui import UIManager
import struct
import socket
import select

class CommunicationManager:

    def __init__(self, game):
        self.fmt = 'I256s'
        self.pack_size = struct.calcsize(self.fmt)
        self.network = NetworkManager()
        self.connect()
        self.game = game
        self.ui = UIManager()

    """ Connect to the game server and handle the initial connection """
    def connect(self):
        self.network.connect()
        message, status = self.__receive_response()
        if status == ResponseType.ERROR:
            print(f"Connection failed: {message}")
            exit(1)
        print(f"Connected!")
        self.__login()

    """ Send a request to the server and receive a response """
    def send_request(self, request_type: RequestType):
        self.network.send(request_type.value)
        print("Request sent to the server.")
        return self.__receive_response()

    """ Login method to handle user login """
    def __login(self):
        while True:
            username = input("Enter username: ")
            if len(username) <= 16:
                username = username.encode('utf-8')
                self.network.send(username)
                message, status = self.__receive_response()
                if status == ResponseType.OK:
                    print(f"Login successful: {message}")
                    break
                else:
                    print(f"Login failed: {message}")

    """ Receive a response from the server, ensuring the data is complete """
    def __receive_response(self):
        try:
            data = self.network.receive(self.pack_size)
            if data is None:
                print("No data received")
                raise ConnectionError("No data received from the server.")
            if len(data) < self.pack_size:
                print(f"Incomplete data received: {len(data)} bytes")
                raise ValueError("Received data is incomplete or malformed.")
            status_code, message = struct.unpack(self.fmt, data)
            message = message.decode('utf-8').rstrip('\0')
            return message, ResponseType(socket.ntohl(status_code))
        except Exception as e:
            print(f"Error receiving response: {e}")
            exit(1)
