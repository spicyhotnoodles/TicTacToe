#include "hash.h"

int hash_fd(int fd) {
    return fd % PLAYER_TABLE_SIZE;
}

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

bool username_exists(const char* username) {
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        if (player_table[i].in_use && strcmp(player_table[i].data.username, username) == 0) {
            return true;
        }
    }
    return false;
}

long random_id(int seed) {
    srand(time(0) + seed);
    return ((rand() % 90000) + 10000); // Random number between 10000 and 99999
}