"""
from pwn import *

REMOTE_NC_CMD    = "nc localhost 6969"    # `nc <host> <port>`

from pwnlib.tubes.tube import tube
tube.s        = tube.send
tube.sa        = tube.sendafter
tube.sl        = tube.sendline
tube.sla    = tube.sendlineafter
tube.r        = tube.recv
tube.ru        = tube.recvuntil
tube.rl        = tube.recvline

def conn():
    return remote(REMOTE_NC_CMD.split()[1], int(REMOTE_NC_CMD.split()[2]))


io = conn()
io.interactive()

from pwn import *

REMOTE_NC_CMD = "nc localhost 6969"  # `nc <host> <port>`

def conn():
    return remote(REMOTE_NC_CMD.split()[1], int(REMOTE_NC_CMD.split()[2]))

io = conn()

# Riceve i dati inviati dal server e li stampa
board = io.recv().decode()
print(board)  # Stampa la tabella del tris ricevuta

# Ora entra in modalità interattiva per continuare a giocare
io.interactive()
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
data = clientSocket.recv(1024)
print( data.decode())
clientSocket.close()
"""