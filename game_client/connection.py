# game_client/connection.py
import socket
from .utils import ServerError
from .config import HOST, PORT
from .protocol import serialize_request, deserialize_response, Response, Request

class GameClient:
    def __init__(self):
        self.sock = socket.create_connection((HOST, PORT))
    
    def send_username(self, username: str):
        self.sock.sendall(username.encode())

    def send_request(self, req: Request) -> Response:
        self.sock.sendall(serialize_request(req))
        data = self._recv_exact(4)
        return deserialize_response(data)

    # Wait for approval from the host of the session
    def wait_for_host(self):
        data = self.sock.recv(1024)
        if not data:
            raise ServerError
        else:
            """
            Hardcoded check. This is wrong and should be changed (Its not scalable)
            Instead we should use an enum code like JOIN_DENY and JOIN_ACCEPT 
            """
            if ('accepted' in data.decode()): 
                return True
            else: 
                return False

    # Prompt the host to accept or deny guest's request to join the game
    def wait_for_guest(self):
        data = self.sock.recv(1024)
        while True:            
            uinput = input(data.decode());
            if uinput.lower() not in ('y','n'):
                print("Invalid input: please try again")
            else:
                break
        self.sock.sendall(uinput.encode())

    def _recv_exact(self, n: int) -> bytes:
        buf = bytearray()
        while len(buf) < n:
            chunk = self.sock.recv(n - len(buf))
            if not chunk:
                raise ConnectionError("Server closed connection")
            buf.extend(chunk)
        return bytes(buf)

    def close(self):
        self.sock.close()
