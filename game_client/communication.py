from .protocol import Status
from .config import SERVER_IP, SERVER_PORT
from typing import Dict, Any
import socket, queue, threading, json

class CommunicationManager:

    def __init__(self, notification_queue):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.message_queue = queue.Queue()
        self.notification_queue = notification_queue
        self.receiver_thread = threading.Thread(target=self.__receiver, daemon=True)
        self.connect()

    """ Connect to the game server """
    def connect(self):
        print(f"Connecting to server at {SERVER_IP}:{SERVER_PORT}...")
        self.socket.connect((SERVER_IP, SERVER_PORT))
        self.receiver_thread.start()
        response = self.receive_message()
        print(response['status'])
        if response['status'] == Status.OK.value:
            print(f"Connected: {response['payload']['message']}")
            self.__login()
        else:
            print(f"Connection failed: {response['payload']['message']}")
            exit(1)

    """ Login method to handle user login """
    def __login(self):
        while True:
            username = input("Enter username: ")
            if len(username) <= 16:
                self.send_message("login", {"username" : username})
                response = self.receive_message()
                if response["status"] == Status.OK.value:
                    print("Login successful!")
                    break
                else:
                    print(f"Login failed: {response['payload']['message']}")
                    continue
            else:
                print("Username must be 16 characters or less. Please try again.")
                continue

    def __receiver(self):
        while True:
            try:
                data = self.socket.recv(1024).decode()
                if not data:
                    print("Connection closed by the server")
                    self.notification_queue.put(None)
                    self.message_queue.put(None)
                    break
                parsed = json.loads(data)
                payload = parsed.get('payload', {})
                if 'notification' in payload:
                    self.notification_queue.put(parsed)
                else:
                    self.message_queue.put(parsed)
            except socket.error as e:
                print(f"Failed to receive data: {e}")
                self.message_queue.put(None)
                self.notification_queue.put(None)
                break
            except json.JSONEncoder as e:
                print(f"Error in decoding json: {e}")
                self.message_queue.put(None)
                self.notification_queue.put(None)
                break
            
    def receive_message(self):
        data = self.message_queue.get()
        if not data:
            print("Error")
            exit(1)
        return data
        

    def send_message(self, method: str, payload: Dict[str, Any] ) -> bool:
        try:
            message = {
                "method": method,
                "payload": payload
            }
            json_message = json.dumps(message)
            self.socket.sendall(json_message.encode())
            return True
        except socket.error as e:
            print(f"Failed to send data: {e}")
            return False
        except json.JSONDecodeError as e:
            print(f"Failed to encode JSON: {e}")
            return False