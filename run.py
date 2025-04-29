"""
import socket

clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverAddress = ('localhost', 6969)
clientSocket.connect(serverAddress)

while True:
    message = clientSocket.recv(1024).decode()
    print(message)

    message = message.lower()

    if "your turn" in message:
        move = input()
        clientSocket.send(move.encode())

    elif "rematch" in message:
        choice = input()
        clientSocket.send(choice.encode())

    elif "win" in message or "lost" in message:
        continue

    elif "draw" in message or "game over" in message:
        break

clientSocket.close()
"""
# run.py
from game_client.client import App

if __name__ == "__main__":
    App().run()
