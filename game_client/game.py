from .ui import UIManager
from .communication import CommunicationManager
from .protocol import RequestType, ResponseType
import threading
from queue import Queue

class GameManager():

    def __init__(self, notification_queue: Queue):
        """ Initialize the GameManager """
        self.notification_queue = notification_queue
        self.hosted_games = [] # List of tuples (game_id, status)
        self.ui = UIManager()
        self.communication = CommunicationManager()

    def new_game(self):
        message, status = self.communication.send_request(RequestType.NEWGAME)
        if status == ResponseType.OK:
            game = (int(message), "Waiting for guest...")
            self.hosted_games.append(game)
            threading.Thread(target=self.wait_for_guest, daemon=True).start()
            self.ui.alert("New game created successfully!")
        else:
            self.ui.alert(f"Failed to create a new game: {message}")
            return

    def join_game(self):
        message, status = self.communication.send_request(RequestType.JOINGAME)
        if status == ResponseType.OK:
            game_list = [line for line in message.split('\n') if line]
            while True:
                input = self.ui.display_list(game_list, title="Available Games", prompt="Select a game to join by index: ")
                if input.isdigit() and 0 < int(input) <= len(game_list):
                    # TODO 
                    # Implement joining game logic
                    return
                else:
                    print("Invalid choice.")
                    continue
        else:
            self.ui.alert(f"Failed to retrieve game list: {message}")
            return

    def wait_for_guest(self):
        # TODO
        """ Next steps:
            1. Listen to the socket for guest joining
            2. Update the game status when a guest joins
            3. Notify the host and guest about the game status using the notification queue
        """

    def my_games(self):
        self.ui.display_list(self.hosted_games, title="Your Active Games", prompt="Press Enter to continue...")

