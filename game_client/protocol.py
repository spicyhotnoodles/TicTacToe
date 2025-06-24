from enum import Enum, auto

class RequestType(Enum):
    NEWGAME = 0
    JOINGAME = 1
    REMATCH = 2
    LOGOUT = 3

class ResponseType(Enum):
    OK = 0
    ERROR = 1
    DENIED = 2
