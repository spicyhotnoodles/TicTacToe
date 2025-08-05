from .game import GameManager
from .ui import UIManager
from queue import Queue

class App:
    def __init__(self):
        print("Initializing the game client...")
        self.notification_queue = Queue()
        self.game = GameManager(self.notification_queue)
        self.ui = UIManager()
        print("Game client initialized successfully.")

    def run(self):
        while True:
            try:
                if not self.notification_queue.empty():
                    game_id, guest_name = self.notification_queue.get()
                    self.ui.alert(f"'{guest_name}' wants to join your game #{game_id}. Head to 'My Games' to start playing!")
                choice = self.ui.menu()
                match choice:
                    case "1":
                        self.game.new_game()
                        pass
                    case "2":
                        self.game.join_game()
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
