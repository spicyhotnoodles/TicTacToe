# game_client/connection.py
import socket
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
    
    # Wait for server to notify for a specific event
    def poll(self):
        data = self.sock.recv(1024)
        if not data:
            print("❌ Error from server: no data received")
        else:
            response = deserialize_response(data)
            if response == Response.GAME_START:
                print("Game started!")
                return
            else:
                print("❌ Error from server: unknown response")

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
