#ifndef HASH_H
#define HASH_H

#include "types.h"

// Hash function to map file descriptor to an index in the player table
int hash_fd(int fd);

// Function to find a player by file descriptor
struct player *player_get(int fd);

// Function to add a new player to the player table
bool player_add(struct player p);

// Function to remove a player from the player table
bool player_remove(int fd);

// Function to check if a username already exists in the player table
bool username_exists(const char* username);

long random_id(int seed);

#endif