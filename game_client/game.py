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

    def __init__(self, notification_queue: Queue):
        """ Initialize the GameManager """
        self.hosted_games = []
        self.ui = UIManager()
        self.communication = CommunicationManager(notification_queue)

    def new_game(self):
        self.communication.send_message("new_game", "null")
        response = self.communication.receive_message()
        if response['status'] == Status.OK.value:
            game_id = response['payload']['game_id']
            game = Game(game_id, None)
            self.hosted_games.append(game)
            self.ui.alert("New game created successfully")

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
                print("List of Available Games:")
                for i, game in enumerate(game_list):
                    print(f"{i}. Game ID: {game['game_id']}, Host: {game['host']}")
                try:
                    uinput = int(input("Select the game you want to join by index: "))
                    game_id = game_list[uinput]['game_id']
                    if (self.__send_join_request(game_id)):
                        # Wait for host
                        # TODO
                        print("Success!")
                        pass
                    else:
                        return
                except ValueError:
                    print("Invalid input! Please try again")
                except IndexError:
                    print("Index is out of game list! Please try again.")

    def my_games(self):
        if not self.hosted_games:
            print("No hosted games.")
        else:
            while True:
                print("My Hosted Games:")
                for i, game in enumerate(self.hosted_games):
                    print(f"{i}. Game ID: {game.id}, Guest: {game.guest}")
                uinput = input("Press 'm' to return to main menu or choose a game by index: ")
                if uinput.lower() == 'm':
                    break
                elif uinput.isdigit():
                    selected_index = int(uinput)
                    if selected_index in range(len(self.hosted_games)):
                        selected_game = self.hosted_games[selected_index]
                        if not selected_game.guest:
                            self.ui.alert("No guest has joined this game yet.")
                            continue
                        else:
                            while True:
                                uinput = input(f"{selected_game.guest} has requested to join this game. Accept? (y/n)")
                                if uinput.lower() == 'y':
                                    # Start game
                                    return
                                elif uinput.lower() == 'n':
                                    # Sends rejection
                                    self.communication.send_message("send_rejection", {"game_id": selected_game.id})
                                else:
                                    print("Invalid input! Please enter 'y' or 'n'.")
                                    continue
                    else:
                        print("Index is out of range! Please try again.")
                else:
                    print("Enter a valid index or 'm' to return to main menu.")

    # Helper functions:
    
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