# game_client/ui.py
import shutil
import os

class UIManager:

    def __init__(self):
        self.cols = shutil.get_terminal_size().columns
        self.menu_options = ["New Game", "Join Game", "Game List", "Credits", "Logout"]

    def clear(self):
        os.system('cls' if os.name == 'nt' else 'clear')

    def title(self):
        art = [
            "  ████████╗██╗ ██████╗    ████████╗ █████╗  ██████╗    ████████╗ ██████╗ ███████╗",
            "  ╚══██╔══╝██║██╔════╝    ╚══██╔══╝██╔══██╗██╔════╝    ╚══██╔══╝██╔═══██╗██╔════╝",
            "     ██║   ██║██║            ██║   ███████║██║            ██║   ██║   ██║█████╗  ",
            "     ██║   ██║██║            ██║   ██╔══██║██║            ██║   ██║   ██║██╔══╝  ",
            "     ██║   ██║╚██████╗       ██║   ██║  ██║╚██████╗       ██║   ╚██████╔╝███████╗",
            "     ╚═╝   ╚═╝ ╚═════╝       ╚═╝   ╚═╝  ╚═╝ ╚═════╝       ╚═╝    ╚═════╝ ╚══════╝" 
        ]
        for line in art:
            print(line.center(self.cols))

    def menu(self):
        self.clear()
        self.title()
        for i, opt in enumerate(self.menu_options, 1):
            print(f"{i}. {opt}".center(self.cols))

    def credits(self):
        self.clear()
        print("Game Client - Credits".center(self.cols))
        print("=" * self.cols)
        print("Developed by: Pietro Ciuci and Michele Esito".center(self.cols))
        print("Version: 1.0.0".center(self.cols))
        print("=" * self.cols)
        input("Press Enter to return to the menu...")
        self.menu()

    def game_list(self, games):
        print("Available Games:".center(self.cols))
        for i, game in enumerate(games, 1):
            print(f"{i}. {game}".center(self.cols))

    def prompt_message(self, message, default_action="Press Enter to continue..."):
        self.clear()
        print(message.center(self.cols))
        uinput = input(default_action)
        self.menu()
        return uinput

