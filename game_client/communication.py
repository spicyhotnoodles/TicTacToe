from .protocol import RequestType, ResponseType
from .game import GameManager
from .network import NetworkManager
from .ui import UIManager
import struct
import socket

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
        message, status = self.receive_response()
        if status == ResponseType.ERROR:
            print(f"Connection failed: {message}")
            exit(1)
        print(f"Connected!")
        self.login()

    """ Login method to handle user login """
    def login(self):
        while True:
            username = input("Enter username: ")
            if len(username) <= 16:
                username = username.encode('utf-8')
                self.network.send(username)
                message, status = self.receive_response()
                if status == ResponseType.OK:
                    print(f"Login successful: {message}")
                    break
                else:
                    print(f"Login failed: {message}")

    """ Send a request to the server and receive a response """
    def send_request(self, request_type: RequestType):
        self.network.send(request_type.value)
        print("Request sent to the server.")
        return self.receive_response()

    """ Receive a response from the server, ensuring the data is complete """
    def receive_response(self):
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

    def handle_request(self, request_type: RequestType, queue):
        message, status_code = self.send_request(request_type)
        if status_code == ResponseType.ERROR:
            raise Exception(f"Server error: {message}")
        else:
            match request_type:
                case RequestType.NEWGAME:
                    self.game.append_hosted_game(message)
                    self.ui.alert("New game created successfully! You can now wait for another player to join or create a new game.")
                case RequestType.JOINGAME:
                    game_list = [line for line in message.split('\n') if line]
                    while True:
                        choice = self.ui.display_list(game_list, title="Available Games", prompt="Select a game to join by index (or 'm' to return to menu): ")
                        if choice.lower() == 'm':
                            break
                        if choice.isdigit() and 0 < int(choice) <= len(game_list):
                            game_id = game_list[int(choice) - 1]
                            print(f"Joining game with ID: {game_id}")
                            break
                        else:
                            print(f"Invalid choice: {choice}. Please select a valid game index or 'm' to return to the menu.")
                            continue
                case RequestType.LOGOUT:
                    print(f"Logout requested successfully: {message}")

    #TODO: Implement the waiting thread
    def wait_for_guest(self, queue):
        # sleep for a while to simulate waiting for a guest
        import time
        time.sleep(5)
        # Notify the user that a guest has joined
        queue.put("guest_joined")
