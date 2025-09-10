#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "hash.h"

cJSON *create_game_list(struct player *player);
int create_game(int fd, struct player *host);
enum GameStatus evaluate_game_state(char board[][3]);
char * print_board(char buffer[], char board[][3]);
void free_game(gpointer data);

#endif