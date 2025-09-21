from .protocol import Status
from .config import SERVER_IP, SERVER_PORT
from typing import Dict, Any
import socket, queue, threading, json, codecs, time

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
        decoder = codecs.getincrementaldecoder('utf-8')()
        buf = ""  # accumulated decoded text (may contain multiple messages)
        jd = json.JSONDecoder()
        while True:
            try:
                chunk = self.socket.recv(4096)
                if not chunk:
                    print("Connection closed by the server")
                    self.notification_queue.put(None)
                    self.message_queue.put(None)
                    break
                # decode incrementally to avoid UTF-8 decode errors on split multibyte sequences
                buf += decoder.decode(chunk)
                # try to extract as many full JSON messages as present in buf
                while buf:
                    try:
                        obj, end = jd.raw_decode(buf)
                    except json.JSONDecodeError:
                        # likely incomplete JSON: wait for more data
                        break
                    # obj is the parsed JSON value, buf[:end] is the exact consumed text
                    message_text = buf[:end]
                    with open("debug_received.json", "w", encoding="utf-8") as f:
                        f.write(f"{time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())} - Received message: \n\n")
                        f.write(message_text + "\n\n")
                    parsed = obj
                    payload = parsed.get('payload', {})
                    if 'notification' in payload:
                        self.notification_queue.put(parsed)
                    else:
                        self.message_queue.put(parsed)
                    # remove the consumed message and any leading whitespace for next iteration
                    buf = buf[end:].lstrip()
            except socket.error as e:
                print(f"Failed to receive data: {e}")
                self.message_queue.put(None)
                self.notification_queue.put(None)
                break
            except Exception as e:
                # unexpected error (rare). Log and break.
                print(f"Receiver error: {e}")
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
        except json.JSONEncoder as e:
            print(f"Failed to encode JSON: {e}")
            return False