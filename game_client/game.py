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
                input = self.ui.display_list(game_list, title="Available Games", prompt="Select a game to join by index (or 'm' to return to menu): ")
                if input.lower() == 'm':
                    return
                elif input.isdigit() and 0 < int(input) <= len(game_list):
                    game_id = game_list[int(input) - 1]
                    game_id = game_id.split()[2].replace(',', '')
                    game_id = int(game_id)
                    self.communication.send_data(game_id)
                    self.ui.display(f"Waiting for host approval...")
                    # TODO: Implement proper waiting mechanism
                    # For now, just return to menu
                    self.ui.alert("Join request sent! Check back later for status.")
                    return
                else:
                    self.ui.alert(f"Invalid choice: {input}. Please select a valid game index or 'm' to return to the menu.")
                    continue
        else:
            self.ui.alert(f"Failed to retrieve game list: {message}")
            return

    def wait_for_guest(self):
        data = self.communication.receive_data()
        game_id, guest = data.decode('utf-8').split(';')
        # update game status 
        for index in range(len(self.hosted_games)):
            if self.hosted_games[index][0] == int(game_id):
                self.hosted_games[index] = (self.hosted_games[index][0], f"{guest} joined")
                break
        self.notification_queue.put((game_id, guest))

    def my_games(self):
        self.ui.display_list(self.hosted_games, title="Your Active Games", prompt="Press Enter to continue...")

