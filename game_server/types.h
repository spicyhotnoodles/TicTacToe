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
#include <cjson/cJSON.h> // For cJSON library
#include <errno.h>
#include <glib.h> // For GHashTable and GList
#include "config.h"

#define BUFFER_SIZE 1024

// HTTP-like status codes
enum StatusCode {
    OK = 200, // Request was successful
    ERROR = 400 // There was an error with the request
};

enum LobbyStatus {
    WAITING_FOR_GUEST, // Game was just created
    WAITING_FOR_HOST, // A guest player requested to join. If the host rejects them the status turns back to WAIT_FOR_GUEST otherwise turns into IN_PROGRESS
    IN_PROGRESS // The game is being played
};

enum GameStatus {
  UNDECIDED, // Game is still ongoing
  DRAW, // The game ended in a draw
  PLAYER1_WINS, // Host wins - X
  PLAYER2_WINS // Guest wins - O
};

/*
    message_t is a structure representing a message, which includes a pointer
    to a string (method) for specifying an RPC method (used only for request messages), 
    an integer (status_code) for indicating the status code (used only for response messages),
    and a pointer to a cJSON object (payload) for holding the message's data.
*/

typedef struct {
    char *method; // RPC method. Optional, for request messages only 
    int status_code; // Status code. Optional, for response messages only
    cJSON *payload; // Contains either argumnet for the RPC (request only) or a message to clarify the status code
} message_t;

// Forward declarations
struct player;

/// @brief Represents a game session between two players.
struct game {
    int id; // Unique game ID
    struct player *host; // Player who created the game
    struct player *guest; // Player who joined the game
    enum LobbyStatus status; // Current status of the game: WAITING_FOR_GUEST, WAITING_FOR_HOST, IN_PROGRESS
    char board[3][3]; // 3x3 game board
    bool host_turn; // true if it's host's turn (X), false if it's guest's turn (O)
};

/// @brief Represents a player connected to the server.
struct player {
    int fd; // File descriptor associated with the player
    char *username; // Player's username
    GList *hosted_game_ids; // List of game IDs the player is hosting
    GList *joined_game_ids; // List of game IDs the player has joined as guest
    
    // The lists above makes the hash table cleanup easier because the lookup is faster
};

extern GHashTable *games; // Declared in main.c
extern GHashTable *players; // Declared in main.c

#endif