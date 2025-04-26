# game_client/client.py
from .ui import clear_screen, print_title, prompt_username, prompt_menu_choice
from .connection import GameClient
from .protocol import Request, Response
from .config import MAX_NAME_LEN
import shutil

MENU = ["New Game", "Join Game", "Credits", "Quit"]

class App:
    def __init__(self):
        clear_screen()
        self.client = GameClient()

    def run(self):
        name = prompt_username(MAX_NAME_LEN)
        self.client.send_gusername(name)

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
        self._handle_resp(resp, "Game created!")

    def _join_game(self):
        resp = self.client.send_request(Request.JOIN_GAME)
        self._handle_resp(resp, "Joined game!")

    def _credits(self):
        clear_screen()
        print("Credits".center(shutil.get_terminal_size().columns))
        print("Made by Michele Esito and Pietro Ciuci!".center(shutil.get_terminal_size().columns))

    def _handle_resp(self, resp: Response, success_msg: str):
        if resp is Response.OK:
            print(success_msg)
        else:
            print("❌ Error from server.")

