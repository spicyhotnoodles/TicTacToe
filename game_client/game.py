from .ui import UIManager
from .communication import CommunicationManager
from .protocol import RequestType, ResponseType
from .network import NetworkManager
import threading

class GameManager():

    def __init__(self):
        """ Initialize the GameManager """
        self.hosted_games = []
        self.ui = UIManager()

    def new_game(self, communication: CommunicationManager, notify_queue=None):
        message, status = communication.send_request(RequestType.NEWGAME)
        if status == ResponseType.OK:
            game_id = int(message)
            self.hosted_games.append(game_id)
            self.ui.alert("New game created successfully! You can now wait for another player to join or create a new game. Please check periodically your active games list to see if a guest has joined.")
            threading.Thread(target=self.wait_for_guest, args=(communication.network, notify_queue), daemon=True).start()
        else:
            self.ui.alert(f"Failed to create a new game: {message}")
            return

    def join_game(self, communication: CommunicationManager, notify_queue=None):
        message, status = communication.send_request(RequestType.JOINGAME)
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
                    communication.network.send(game_id)
                    self.ui.display(f"Waiting for host approval...")
                    while True:
                        pass
                    break
                else:
                    self.ui.alert(f"Invalid choice: {input}. Please select a valid game index or 'm' to return to the menu.")
                    continue
        else:
            self.ui.alert(f"Failed to retrieve game list: {message}")
            return

    def wait_for_guest(self, network: NetworkManager, notify_queue):
        data = network.receive()
        game_id, guest_name = data.decode('utf-8').split(';')
        game_id = int(game_id)
        if game_id in self.hosted_games:
            # Notify the main thread about the guest request
            notify_queue.put((game_id, guest_name))

    def my_games(self):
        self.ui.display_list(self.hosted_games, title="Your Active Games", prompt="Press Enter to continue...")

