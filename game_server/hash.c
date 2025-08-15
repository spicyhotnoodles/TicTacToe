#include "hash.h"

// Probing technique used is linear probing (for simplicity)

/// @brief Hashes a file descriptor to an index in the player table.
/// @param fd The file descriptor to hash.
/// @return The hashed index.
int hash_fd(int fd) {
    return fd % PLAYER_TABLE_SIZE;
}

/// @brief Adds a player to the player table.
/// @param p The player to add.
/// @return true if the player was added successfully, false otherwise.
bool player_add(struct player p) {
    int index = hash_fd(p.fd);
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        int try = (index + i) % PLAYER_TABLE_SIZE;
        if (!player_table[try].in_use) {
            player_table[try].fd = p.fd;
            player_table[try].data = p;
            player_table[try].in_use = true;
            return true;
        }
    }
    return false; // Table full
}

/// @brief Retrieves a player from the player table.
/// @param fd The file descriptor of the player to retrieve.
/// @return A pointer to the player structure if found, NULL otherwise.
struct player* player_get(int fd) {
    int index = hash_fd(fd);
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        int try = (index + i) % PLAYER_TABLE_SIZE;
        if (player_table[try].in_use && player_table[try].fd == fd) {
            return &player_table[try].data;
        }
    }
    return NULL;
}

/// @brief Removes a player from the player table.
/// @param fd The file descriptor of the player to remove.
/// @return true if the player was removed, false otherwise.
bool player_remove(int fd) {
    int index = hash_fd(fd);
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        int try = (index + i) % PLAYER_TABLE_SIZE;
        if (player_table[try].in_use && player_table[try].fd == fd) {
            player_table[try].in_use = false;
            return true;
        }
    }
    return false;
}

/// @brief Checks if a username already exists in the player table.
/// @param username The username to check for existence.
/// @return true if the username exists, false otherwise.
bool username_exists(const char* username) {
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        if (player_table[i].in_use && strcmp(player_table[i].data.username, username) == 0) {
            return true;
        }
    }
    return false;
}

/// @brief Generates a random 5-digit number (between 10000 and 99999) based on a given integer seed. It initializes the random number generator using the current time offset by the seed value to ensure variability.
/// @param seed The seed value for random number generation
/// @return A random 5-digit number
long random_id(int seed) {
    srand(time(0) + seed);
    return ((rand() % 90000) + 10000);
}