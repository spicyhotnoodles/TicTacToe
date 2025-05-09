from enum import IntEnum

class Request(IntEnum):
    NEW_GAME   = 0
    JOIN_GAME  = 1
    LEAVE_GAME = 2

class Response(IntEnum):
    OK    = 0
    ERROR = 1

def serialize_request(req: Request) -> bytes:
    return req.to_bytes(4, "big")

def deserialize_response(data: bytes) -> Response:
    code = int.from_bytes(data, "big")
    return Response(code)