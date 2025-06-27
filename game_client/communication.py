from .protocol import RequestType, ResponseType
from .game import GameManager
from .ui import UIManager
import struct
import threading
import socket

class CommunicationManager:

    def __init__(self, game):
        self.fmt = 'I256s'
        self.pack_size = struct.calcsize(self.fmt)
        self.game = game
        self.ui = UIManager()

    def send_request(self, request_type: RequestType, network):
        network.send(request_type.value.to_bytes(4, 'big'))
        print("Request sent to the server.")
        return self.receive_response(network)

    def receive_response(self, network):
        try:
            data = network.receive(self.pack_size)
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

    def handle_request(self, request_type: RequestType, network):
        message, status_code = self.send_request(request_type, network)
        if status_code == ResponseType.ERROR:
            raise Exception(f"Server error: {message}")
        else:
            match request_type:
                case RequestType.NEWGAME:
                    # print(f"New game created successfully with ID: {message}")
                    self.game.add_game(message)
                    self.ui.prompt_message("New game created successfully! You can now wait for another player to join or create a new game.")
                    # Create a thread to wait for a guest without blocking the main thread
                    # threading.Thread(target=self.wait_for_guest, args=(self.ui,), daemon=True).start()
                    #TODO: Implement waiting for a guest to join with a non blocking thread
                case RequestType.JOINGAME:
                    game_list = [line for line in message.split('\n') if line]
                    while True:
                        self.ui.clear()
                        self.ui.game_list(game_list)
                        choice = input("Select a game to join by index (or 'm' to menu): ")
                        if choice.lower() == 'm':
                            self.ui.menu()
                            return
                        if choice.isdigit() and 0 < int(choice) <= len(game_list):
                            game_id = game_list[int(choice) - 1]
                            #TODO: Implement joining the game with the selected ID
                            print(f"Joining game with ID: {game_id}")
                            self.ui.menu()
                            break
                        else:
                            print(f"Invalid choice: {choice}. Please select a valid game index or 'm' to return to the menu.")
                case RequestType.LOGOUT:
                    #TODO: Implement logout functionality
                    print(f"Logout requested successfully: {message}")

    #TODO: Implement the waiting thread
    def wait_for_guest(self):
        # sleep for a while to simulate waiting for a guest
        import time
        time.sleep(5)
        # Notify the user that a guest has joined
        self.ui.prompt_message("A guest has joined your game! You can now start playing.", "Press Enter to continue...")
