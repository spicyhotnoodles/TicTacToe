# game_client/ui.py
import shutil
import os

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

cols = shutil.get_terminal_size().columns

def print_title():
    art = [
        "  ████████╗██╗ ██████╗    ████████╗ █████╗  ██████╗    ████████╗ ██████╗ ███████╗",
        "  ╚══██╔══╝██║██╔════╝    ╚══██╔══╝██╔══██╗██╔════╝    ╚══██╔══╝██╔═══██╗██╔════╝",
        "     ██║   ██║██║            ██║   ███████║██║            ██║   ██║   ██║█████╗  ",
        "     ██║   ██║██║            ██║   ██╔══██║██║            ██║   ██║   ██║██╔══╝  ",
        "     ██║   ██║╚██████╗       ██║   ██║  ██║╚██████╗       ██║   ╚██████╔╝███████╗",
        "     ╚═╝   ╚═╝ ╚═════╝       ╚═╝   ╚═╝  ╚═╝ ╚═════╝       ╚═╝    ╚═════╝ ╚══════╝" 
    ]
    for line in art:
        print(line.center(cols))

def prompt_username(max_len: int) -> str:
    while True:
        name = input("Enter a username: ")
        if not name:
            print("Username cannot be empty.")
        elif len(name) > max_len:
            print(f"Username too long. Max {max_len} chars.")
        else:
            return name

def prompt_menu_choice(options: list[str]) -> str:
    for i, opt in enumerate(options, 1):
        print(f"{i}. {opt}".center(cols))
    return input("Choice: ")
