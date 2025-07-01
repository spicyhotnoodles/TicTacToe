from .communication import CommunicationManager
from .game import GameManager
from .ui import UIManager
from queue import Queue

class App:
    def __init__(self):
        print("Initializing the game client...")
        self.queue = Queue()
        self.game = GameManager()
        self.ui = UIManager()
        self.communication = CommunicationManager(self.game)
        print("Game client initialized successfully.")

    def run(self):
        while True:
            try:
                # 🔔 Check for guest join notifications
                while not self.queue.empty():
                    game_id, guest_name = self.queue.get()
                    self.ui.alert(f"'{guest_name}' wants to join your game {game_id}.", "Accept (y/n)? ")
                choice = self.ui.menu()
                match choice:
                    case "1":
                        self.game.new_game(self.communication, self.queue)
                        pass
                    case "2":
                        self.game.join_game(self.communication, self.queue)
                        pass
                    case "3":
                        self.game.my_games()
                        pass
                    case "4":
                        self.ui.credits()
                        pass
                    case "5":
                        print("Logging out...")
                        break
                    case _:
                        print("Invalid choice.")
            except KeyboardInterrupt:
                print("\nExiting the game client...")
                break
