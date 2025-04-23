import shutil
import socket
import os # for clearing the screen
import time # for sleep

title = ["  ████████╗██╗ ██████╗    ████████╗ █████╗  ██████╗    ████████╗ ██████╗ ███████╗",
         "  ╚══██╔══╝██║██╔════╝    ╚══██╔══╝██╔══██╗██╔════╝    ╚══██╔══╝██╔═══██╗██╔════╝",
         "     ██║   ██║██║            ██║   ███████║██║            ██║   ██║   ██║█████╗  ",
         "     ██║   ██║██║            ██║   ██╔══██║██║            ██║   ██║   ██║██╔══╝  ",
         "     ██║   ██║╚██████╗       ██║   ██║  ██║╚██████╗       ██║   ╚██████╔╝███████╗",
         "     ╚═╝   ╚═╝ ╚═════╝       ╚═╝   ╚═╝  ╚═╝ ╚═════╝       ╚═╝    ╚═════╝ ╚══════╝"
         ]
menu = ["1. New Game", "2. Join Game", "3. Credits", "4. Quit"]
host = "localhost"
port = 8080
columns = shutil.get_terminal_size().columns
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def main():
    establish_connection()
    username_setup()
    menu_loop()

def establish_connection():
    try:
        sock.connect((host, port))
    except ConnectionRefusedError:
        print("Connection issue. Server may be offline or maximum connections reached.")
        exit(1)

def username_setup():
    username = input("Enter a username: ")
    sock.sendall(username.encode())

def send_request(request):
    socket.sendall(request.encode())
    response = socket.recv(1024).decode()
    socket.close()
    return response

def handle_response(response):
    status, _, data = response.partition('|')
    if status == "OK":
        print("Success:", data)
    elif status == "ERROR":
        print("Error:", data)
    else:
        print("Unknown response:", response)

def print_box(symbol, text):
    if len(text) + 4 > columns:
        text = text[:columns - 4]  # truncate text if it's too long

    # Top and bottom border
    border = symbol * columns

    # Calculate centered text line
    padding_total = columns - len(text) - 2  # minus 2 for symbol borders
    padding_left = padding_total // 2
    padding_right = padding_total - padding_left
    text_line = symbol + ' ' * padding_left + text + ' ' * padding_right + symbol

    print(border)
    print(text_line)
    print(border)

def clear():
    os.system('clear' if os.name == 'posix' else 'cls')

def menu_loop():
    while True:
        clear()
        for string in title:
            print(string.center(columns))
        print("\n\n\n")
        for voice in menu:
            print(voice.center(columns))
        choice = input("Enter your choice: ")
        clear()
        match choice:
            case "1":
                response = send_request("NEW_GAME")
                if handle_response(response):
                    print_box("=", "Game started successfully!")
                else:
                    raise Exception("Failed to create a new game.")
            case "2":
                response = send_request("JOIN_GAME")
                if handle_response(response):
                    print_box("=", "Joined game successfully!")
                else:
                    raise Exception("Failed to join the game.")
            case "3":
                print("Credits".center(columns))
                print("This game was made by Pietro!".center(columns))
                input("Press Enter to continue...")
            case "4":
                print("Thanks for playing!".center(columns))
                time.sleep(2)
                clear()
                exit(0)

main()