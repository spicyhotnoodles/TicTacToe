# game_client/ui.py
import shutil
import os
import re


# Define colors using ANSI escape codes
BLUE = "\033[1;36m"  # Blue for X
RED = "\033[91m"   # Red for O
RESET = "\033[0m"  # Reset color to default
ANSI_ESCAPE = re.compile(r'\x1b\[[0-9;]*m')

class UIManager:

    def __init__(self):
        self.cols = shutil.get_terminal_size().columns
        self.menu_options = ["New Game", "Join Game", "My Games", "Credits", "Logout"]

    def __clear(self):
        os.system('cls' if os.name == 'nt' else 'clear')

    def __title(self):
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
        print("\n")

    def menu(self):
        self.__clear()
        self.__title()
        return self.display_list(self.menu_options, prompt="Enter your choice: ")

    def credits(self):
        self.__clear()
        print("Game Client - Credits".center(self.cols))
        print("=" * self.cols)
        print("Developed by: Pietro Ciuci and Michele Esito".center(self.cols))
        print("Version: 1.0.0".center(self.cols))
        print("=" * self.cols)
        input("Press Enter to return to the menu...")

    def alert(self, message, default_action="Press Enter to continue..."):
        self.__clear()
        print(message.center(self.cols))
        uinput = input(default_action)
        return uinput
    
    def display(self, message):
        self.__clear()
        for line in message.splitlines():
            # Strip ANSI codes for length calculation
            visible_line = ANSI_ESCAPE.sub('', line)
            padding = (self.cols - len(visible_line)) // 2
            print(' ' * max(padding, 0) + line)
    
    def display_list(self, items, title=None, prompt="Press Enter to continue..."):
        if title:
            self.__clear()
            print(title.center(self.cols))
            print() # Add a little space after the title
        if not items:
            print("No items available.".center(self.cols))
        else:
            # Create the full strings first to measure them
            formatted_items = [f"{i}. {item}" for i, item in enumerate(items, 1)]
            # Find the length of the longest item to determine the block's width
            if not formatted_items: # Handle case where items is empty
                max_width = 0
            else:
                max_width = max(len(s) for s in formatted_items)
            # Calculate the left padding to center the entire block
            padding = " " * ((self.cols - max_width) // 2)
            # Print each item with the same calculated padding
            for item_line in formatted_items:
                print(f"{padding}{item_line}")
        return input(f"\n{prompt}") # Add a newline for better spacing

    def color_board(self, board: str) -> str:
        # Replace each character 'X' with blue and 'O' with red, and reset others
        board_colored = ""
        for char in board:
            if char == 'X':
                board_colored += f"{BLUE}{char}{RESET}"
            elif char == 'O':
                board_colored += f"{RED}{char}{RESET}"
            else:
                board_colored += char  # For numbers, leave them as they are
        return board_colored