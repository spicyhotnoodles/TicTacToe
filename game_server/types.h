#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h> // For bool type
#include <string.h> // For strncpy
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#include "config.h"

// Ensure structure is tightly packed (no padding)
#pragma pack(push, 1)
struct packet {
    int status_code;
    char message[MAX_MSG_LEN];
};
#pragma pack(pop)

// Forward declarations
struct player;

struct game {
    int id;
    struct player *host;
    struct player *guest;
};

struct player {
    int fd;
    char username[17]; // 16 characters + null terminator
    struct game *games[MAX_GAMES_PER_PLAYER];
};

struct player_table_entry {
    int fd;
    struct player data;
    bool in_use;
};

enum Requests {
    NEWGAME,
    JOINGAME,
    REMATCH,
    LOGOUT
};

enum StatusCode {
    OK,
    ERROR,
    DENIED
};

extern int ngames; // Global variable to track the number of games
extern int nfds; // Number of file descriptors currently in use
extern int nplayers; // Number of players currently connected
extern int ngames; // Number of games currently active
extern struct player_table_entry player_table[PLAYER_TABLE_SIZE];



#endif