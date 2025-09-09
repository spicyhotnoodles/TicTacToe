from .ui import UIManager
from .communication import CommunicationManager
from .protocol import Status
from queue import Queue
from dataclasses import dataclass

@dataclass
class Game:
    id: int
    guest: str

class GameManager():

    def __init__(self):
        """ Initialize the GameManager """
        self.hosted_games = []
        self.ui = UIManager()
        self.notification_queue = Queue()
        self.communication = CommunicationManager(self.notification_queue)

    def new_game(self):
        self.communication.send_message("new_game", "null")
        response = self.communication.receive_message()
        if response['status'] == Status.OK.value:
            game_id = response['payload']['game_id']
            game = Game(game_id, None)
            self.hosted_games.append(game)
            self.ui.alert("New game created successfully")
        else:
            self.ui.alert(f"Error: {response['payload']['message']}")

    """ List of games message format
    
    {
        "status" : 200,
        "payload" : {
            "games_list" : [
                {
                    "game_id" : "12345",
                    "host" : "player_1"
                },
                {
                    "game_id : "67890",
                    "host" : "player_2"
                },
                ...
            ]
        }
    }

    """

    def join_game(self):
        game_list = self.__get_game_list()
        if game_list:
            while True:
                items = [f"Game ID: {game['game_id']}, Host: {game['host']}" for game in game_list]
                uinput = self.ui.display_list(items, title="List of Available Games", prompt="Select the game you want to join by index (0 to main menu): ")
                try:
                    idx = int(uinput) - 1
                    if idx < 0:
                        break
                    game_id = game_list[idx]['game_id']
                    if self.__send_join_request(game_id):
                        self.ui.display("Join request sent successfully. Waiting for host approval...")
                        response = self.communication.receive_message()
                        if response['status'] == Status.OK.value:
                            self.__play_game(game_id)
                            return
                        else:
                            self.ui.alert(f"{response['payload']['message']}")
                except (ValueError, IndexError):
                    self.ui.alert("Invalid input! Please try again")

    def my_games(self):
        if not self.hosted_games:
            self.ui.alert("No hosted games.")
            return
        self._process_notifications()
        while True:
            uinput = self._print_hosted_games()
            if uinput == '0':
                break
            if not uinput.isdigit():
                self.ui.alert("Invalid input! Please try again")
                continue
            idx = int(uinput) - 1
            if idx < 0 or idx >= len(self.hosted_games):
                self.ui.alert("Index is out of game list! Please try again.")
                continue
            selected_game = self.hosted_games[idx]
            self._handle_guest(selected_game)
            break

    # Helper functions:
    
    def _process_notifications(self):
        while not self.notification_queue.empty():
            notification = self.notification_queue.get()
            game_id = notification['payload']['notification']['game_id']
            guest_user = notification['payload']['notification']['user']
            self.ui.alert(f"User {guest_user} would like to join your game #{game_id}")
            for game in self.hosted_games:
                if game.id == int(game_id):
                    game.guest = guest_user
                    break
    
    def _print_hosted_games(self):
        items = [f"Game ID: {game.id}, Guest: {game.guest}" for game in self.hosted_games]
        return self.ui.display_list(items, title="My Hosted Games", prompt="Select a game by index (0 to main menu): ")
    
    def _handle_guest(self, selected_game):
        if not selected_game.guest:
            self.ui.alert("No guest has joined this game yet.")
            return
        while True:
            uinput = self.ui.alert(
                f"{selected_game.guest} has requested to join this game.",
                default_action="Accept? (y/n) "
            )
            if uinput.lower() == 'y':
                self.communication.send_message("start_game", {"game_id" : selected_game.id})
                if (self.__play_game(selected_game.id)):
                    self.hosted_games.remove(selected_game)
                else:
                    selected_game.guest = None
                return
            elif uinput.lower() == 'n':
                self.communication.send_message("send_join_rejection", {"game_id": selected_game.id})
                response = self.communication.receive_message()
                if response['status'] == Status.OK.value:
                    selected_game.guest = None
                    self.ui.alert("Rejection sent successfully.")
                else:
                    self.ui.alert(f"Error: {response['payload']['message']}")
                return
            else:
                print("Invalid input! Please enter 'y' or 'n'.")

    def __get_game_list(self):
        self.communication.send_message("get_games_list", "null")
        response = self.communication.receive_message()
        if response['status'] == Status.OK.value:
            game_list = response['payload']['games_list']
            return game_list # List is a dictionary
        else:
            self.ui.alert(f"Error: {response['payload']['message']}")
            return None

    def __send_join_request(self, game_id) -> bool:
        self.communication.send_message("send_join_request", {"game_id" : game_id})
        response = self.communication.receive_message()
        if response['status'] == Status.OK.value:
            return True
        else:
            self.ui.alert(f"Error: {response['payload']['message']}")
            return False
    
    def __play_game(self, game_id):
        while True:
            # Receive game infos
            self.ui.display("Waiting for opponent...")
            data = self.communication.receive_message()
            if data['status'] == Status.OK.value:
                # Check if game is over after opponent move
                if data['payload']['game_state'] != "Game Over":
                    # Make your move
                    while True:
                        self.ui.display(self.ui.color_board(data['payload']['board']))
                        uinput = input("Its your turn, enter a move (1-9): ")
                        self.communication.send_message("make_move", {"game_id" : game_id, "move" : int(uinput)})
                        response = self.communication.receive_message()
                        if response['status'] != Status.ERROR.value:
                            # Check if game is over after own move
                            if "game_state" in response['payload'] and response['payload']['game_state'] == "Game Over":
                                self.ui.alert(response['payload']['result'])
                                return True
                            break
                        else:
                            self.ui.alert(response['payload']['message'])
                else:
                    self.ui.alert(data['payload']['result'])
                    return True
            else:
                self.ui.alert(f"Error: {data['payload']['message']}")
                return False