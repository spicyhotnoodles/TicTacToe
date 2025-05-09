#ifndef types_h
#define types_h

#define MAX_PLAYERS 50
#define MAX_GAMES 25

#define SIZE 3

struct player {
    int fd;
    char username[16];
};

extern struct player players[MAX_PLAYERS];

typedef enum{
    inProgress = 0,
    waiting = 1,
    newCreation = 2
}gameStatus;

typedef enum{
    win = 0,
    lose = 1,
    draw = 2,
}result;

typedef struct {
    gameStatus status;
    result res;
    int match_id;
    struct player *host;
    struct player *guest;
    char board[SIZE][SIZE];
}game;

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