# game_client/client.py
from .ui import clear_screen, print_title, prompt_username, prompt_menu_choice
from .connection import GameClient
from .protocol import Request, Response
import struct
from .config import MAX_NAME_LEN
from .utils import ServerError
import shutil
import re

MENU = ["New Game", "Join Game", "Credits", "Quit"]

class App:
    def __init__(self):
        clear_screen()
        self.client = GameClient()

    def run(self):
        name = prompt_username(MAX_NAME_LEN)
        self.client.send_username(name)
        while True:
            clear_screen()
            print_title()
            choice = prompt_menu_choice(MENU)
            match choice:
                case "1":
                    self._new_game()
                case "2":
                    self._join_game()
                case "3":
                    self._credits()
                case "4":
                    break
            input("Press Enter to continue…")
        self.client.close()

    def _new_game(self):
        resp = self.client.send_request(Request.NEW_GAME)
        try:
            self._handle_resp(resp, "Game created!")
            print("Waiting for players to join…")
            self.client.wait_for_guest()
        except ServerError:
            print("Exception occurred: error from server!")


    def _join_game(self):
        resp = self.client.send_request(Request.JOIN_GAME)
        try:
            self._handle_resp(resp, "Games are available!")
            raw = self.client.sock.recv(4096).decode()
            games = re.findall(r"Game ID: (\d+) \| Host: (\w+)", raw)
            print("Available Games:")
            for i, (game_id, host) in enumerate(games):
                print(f"{i + 1}. Game ID: {game_id} | Host: {host}")
            uinput = input("Select a game by index: ")
            while True:
                if not uinput.isdigit() or int(uinput) < 1 or int(uinput) > len(games):
                    print("Invalid input. Please enter a valid index.")
                    pass
                else:
                    break
            game_id = games[int(uinput) - 1][0]  # Get the selected game ID
            print("Selected: ", game_id)
            self.client.sock.sendall(struct.pack(">I", int(game_id))) # Send game ID in binary format
            printf("Waiting for host to approve the request...")
            if self.client.wait_for_host():
                print("Host accepted request!")
            else:
                print("Host denied request")
        except ServerError:
            print("Exception occurred: error from server!")

    def _credits(self):
        clear_screen()
        print("Credits".center(shutil.get_terminal_size().columns))
        print("Made by Michele Esito and Pietro Ciuci!".center(shutil.get_terminal_size().columns))

    def _handle_resp(self, resp: Response, success_msg: str):
        if resp is Response.OK:
            print(success_msg)
        else:
            raise ServerError
