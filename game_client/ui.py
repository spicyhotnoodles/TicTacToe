# game_client/ui.py
import shutil
import os

class UIManager:

    def __init__(self):
        self.cols = shutil.get_terminal_size().columns
        self.menu_options = ["New Game", "Join Game", "My Games", "Credits", "Logout"]

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
        return self.display_list(self.menu_options, prompt="Enter your choice: ")

    def credits(self):
        self.clear()
        print("Game Client - Credits".center(self.cols))
        print("=" * self.cols)
        print("Developed by: Pietro Ciuci and Michele Esito".center(self.cols))
        print("Version: 1.0.0".center(self.cols))
        print("=" * self.cols)
        input("Press Enter to return to the menu...")

    def alert(self, message, default_action="Press Enter to continue..."):
        self.clear()
        print(message.center(self.cols))
        uinput = input(default_action)
        return uinput
    
    def display(self, message):
        self.__clear()
        print(message.center(self.cols))
    
    def display_list(self, items, title=None, prompt="Press Enter to continue..."):
        if title:
            self.clear()
            print(title.center(self.cols))
        if not items:
            print("No items available.".center(self.cols))
        else:
            for i, item in enumerate(items, 1):
                print(f"{i}. {item}".center(self.cols))
        return input(prompt)