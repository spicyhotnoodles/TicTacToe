from .game import GameManager
from .ui import UIManager

class App:
    def __init__(self):
        print("Initializing the game client...")
        self.game = GameManager()
        self.ui = UIManager()
        print("Game client initialized successfully.")

    def run(self):
        while True:
            try:
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
