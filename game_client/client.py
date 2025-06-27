from .network import NetworkManager
from .communication import CommunicationManager
from .protocol import RequestType, ResponseType
from .game import GameManager
from .ui import UIManager

class App:

    def __init__(self):
        print("Initializing the game client...")
        self.network = NetworkManager()
        self.game = GameManager()
        self.communication = CommunicationManager(self.game)
        self.ui = UIManager()
        self.network.connect(self.communication)
        print("Game client initialized successfully.")

    def run(self):
        self.ui.menu()
        while True:
            choice = input("Enter your choice: ")
            match choice:
                case "1":
                    try:
                        self.communication.handle_request(RequestType.NEWGAME, self.network)
                    except Exception:
                        pass
                case "2":
                    try:
                        self.communication.handle_request(RequestType.JOINGAME, self.network)
                    except Exception:
                        pass
                case "3":
                    self.game.list_games()
                case "4":
                    self.ui.credits()
                case "5":
                    print("Logging out...")
                    self.communication.handle_request(RequestType.LOGOUT, self.network)
                    break
                case _:
                    print("Invalid choice, please try again.")    