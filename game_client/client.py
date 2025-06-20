from .network import NetworkManager
from .communication import CommunicationManager
from .protocol import RequestType, ResponseType
from .ui import UIManager

class App:

    def __init__(self):
        print("Initializing the game client...")
        self.network = NetworkManager()
        self.communication = CommunicationManager()
        self.ui = UIManager()
        self.network.connect(self.communication)
        print("Game client initialized successfully.")

    def run(self):
        self.ui.clear()
        self.ui.title()
        self.ui.menu()
        while True:
            choice = input("Enter your choice: ")
            match choice:
                case "1":
                    self.communication.send_request(RequestType.NEWGAME, self.network)
                case "2":
                    self.communication.send_request(RequestType.JOINGAME, self.network)
                case "3":
                    # Placeholder for game list functionality
                    pass
                case "4":
                    self.ui.credits()
                    pass
                case "5":
                    print("Logging out...")
                    self.communication.send_request(RequestType.LOGOUT, self.network)
                    break
                case _:
                    print("Invalid choice, please try again.")    