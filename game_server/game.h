#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "hash.h"

struct game *get_game(int game_id);

cJSON *create_game_list(struct player *player);

int create_game(int fd, struct player *host);

void start_game(int fd, struct player *guest, int game_index);

/// @brief If a player is playing a game, lock all other games for that player.
/// @param player The player whose games should be locked.
/// @note This function is used to prevent a player from playing multiple games at the same time
void lock_games(struct player *player);

#endif