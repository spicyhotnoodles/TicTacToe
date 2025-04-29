#ifndef types_h
#define types_h

#define MAX_PLAYERS 50
#define MAX_GAMES 25

#define SIZE 3


typedef struct {
    int playerfd[2];
    char board[SIZE][SIZE];
} Match;

struct player {
    int fd;
    char username[16];
};

struct game {
    int match_id;
    struct player host;
    struct player *guest;
};

enum Request {
    NEW_GAME = 0,
    JOIN_GAME = 1,
    LEAVE_GAME = 2
};

enum Response {
    OK = 0,
    ERROR = 1
};

#endif