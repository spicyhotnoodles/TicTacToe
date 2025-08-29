#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h> // For bool type
#include <string.h> // For strncpy
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#include <cjson/cJSON.h>
#include <errno.h>
#include "config.h"

#define BUFFER_SIZE 1024

// HTTP-like status codes
enum StatusCode {
    OK = 200,
    ERROR = 400
};

enum LobbyStatus {
    WAITING_FOR_GUEST, // Game was just created
    WAITING_FOR_HOST, // A guest player requested to join. If the host rejects them the status turns back to WAIT_FOR_GUEST otherwise turns into IN_PROGRESS
    IN_PROGRESS // The game is being played
};

enum GameStatus {
  UNDECIDED,
  DRAW,
  PLAYER1_WINS,
  PLAYER2_WINS
};

/*
    message_t is a structure representing a message, which includes a pointer
    to a string (method) for specifying an RPC method (used only for request messages), 
    an integer (status_code) for indicating the status code (used only for response messages),
    and a pointer to a cJSON object (payload) for holding the message's data.
*/

typedef struct {
    char *method; // RPC method. Optional, for request messages only 
    int status_code;
    cJSON *payload; // Contains either argumnet for the RPC (request only) or a message to clarify the status code
} message_t;

// Forward declarations
struct player;

struct game {
    int id;
    struct player *host;
    struct player *guest;
    enum LobbyStatus status;
    char board[3][3];
    bool host_turn;
};

struct player {
    int fd;
    char username[17]; // 16 characters + null terminator
    struct game *games[MAX_GAMES_PER_PLAYER];
    int ngames; // Number of games the player is involved in
};

struct player_table_entry {
    int fd;
    struct player data;
    bool in_use;
};

extern int ngames; // Global variable to track the number of games
extern int nfds; // Number of file descriptors currently in use
extern int nplayers; // Number of players currently connected
extern int ngames; // Number of games currently active
extern struct player_table_entry player_table[PLAYER_TABLE_SIZE];
extern struct game *games[MAX_GAMES]; // Array to hold active games
extern struct player_table_entry player_table[PLAYER_TABLE_SIZE];
extern struct pollfd fds[MAX_PLAYERS + 1]; // +1 for the server
extern int nfds; // Number of file descriptors currently in use
extern int nplayers; // Number of players currently connected
extern int ngames; // Number of games currently active

#endif