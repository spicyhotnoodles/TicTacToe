from .communication import CommunicationManager
from .protocol import RequestType
from .game import GameManager
from .ui import UIManager
from queue import Queue, Empty
import sys
import select
import tty
import termios

"""def input():
    return select.select([sys.stdin], [], [], 0) == ([sys.stdin], [], [])"""

class App:

    def __init__(self):
        print("Initializing the game client...")
        self.game = GameManager()
        self.communication = CommunicationManager(self.game)
        self.ui = UIManager()
        self.queue = Queue()
        print("Game client initialized successfully.")

    def run(self):
        while True:
            try:
                choice = self.ui.menu()
                match choice:
                    case "1":
                        self.communication.handle_request(RequestType.NEWGAME, self.queue)
                    case "2":
                        self.communication.handle_request(RequestType.JOINGAME, self.queue)
                    case "3":
                        self.ui.display_list(self.game.hosted_games, title="Your Active Games", prompt="Press Enter to continue...")
                    case "4":
                        self.ui.credits()
                    case "5":
                        print("Logging out...")
                        self.communication.handle_request(RequestType.LOGOUT)
                        break
                    case _:
                        print("Invalid choice, please try again.")
            except KeyboardInterrupt:
                print("\nExiting the game client...")
                break
            finally:
                pass