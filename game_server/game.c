#include "game.h"

int get_game_index(int game_id) {
    for (int i = 0; i < ngames; i++) {
        if (games[i].id == game_id) {
            return i;
        }
    }
    return -1; // Game not found
}

char *get_game_list(struct player *player) {
    static char game_list[MAX_MSG_LEN];
    game_list[0] = '\0'; // Initialize the string
    for (int i = 0; i < ngames; i++) {
        if (games[i].guest == NULL && games[i].host != player) { // Only list games without guests
			char game_info[64];
			snprintf(game_info, sizeof(game_info), "Game ID: %d, Host: %s\n", games[i].id, games[i].host->username);
			strncat(game_list, game_info, sizeof(game_list) - strlen(game_list) - 1);
		}
    }
    return game_list;
}