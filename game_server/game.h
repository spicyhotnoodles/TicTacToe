#ifndef GAME_H
#define GAME_H

#include "types.h"

/// @brief Get the index of a game by its ID.
/// @param game_id The ID of the game to find.
/// @return The index of the game in the global games array, or -1 if the game is not found.
int get_game_index(int game_id);

/// @brief Get a list of games available for the player to join.
/// @param player Player requesting the game list.
/// @return A string containing the list of games, formatted as "Game ID: <id>, Host: <username>\n" if there are games available (without guests and not hosted by the player), and an empty string if no games are available.
char *get_game_list(struct player *player);

void create_game(int fd);

#endif