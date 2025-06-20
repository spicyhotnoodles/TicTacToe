from .protocol import RequestType, ResponseType
import struct
import socket

class CommunicationManager:

    def __init__(self):
        self.fmt = 'I256s'
        self.pack_size = struct.calcsize(self.fmt)

    def send_request(self, request_type: RequestType, network):
        match request_type:
            case RequestType.NEWGAME:
                # Dummy logic for starting a new game
                pass
            case RequestType.JOINGAME:
                # Dummy logic for joining a game
                pass
            case RequestType.REMATCH:
                # Dummy logic for rematching
                pass
            case RequestType.LOGOUT:
                # Dummy logic for logging out
                pass
            case _:
                print(f"Unsupported request type: {request_type}")

    def receive_response(self, network):
        try:
            data = network.receive(self.pack_size)
            if data is None:
                print("No data received")
                raise ConnectionError("No data received from the server.")
            if len(data) < self.pack_size:
                print(f"Incomplete data received: {len(data)} bytes")
                raise ValueError("Received data is incomplete or malformed.")
            status_code, message = struct.unpack(self.fmt, data)
            message = message.decode('utf-8').rstrip('\0')
            return message, ResponseType(socket.ntohl(status_code))
        except Exception as e:
            print(f"Error receiving response: {e}")
            exit(1)