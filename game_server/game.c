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

int create_game(int fd, struct player *host) {
    struct game new_game;
    new_game.id = random_id(fd); // Generate a unique game ID
    new_game.host = host; // Set the host player
    new_game.guest = NULL; // Initially no guest
    new_game.status = WAITING_FOR_GUEST; // Set initial status
    games[ngames] = new_game; // Add to the games array
    host->games[host->ngames++] = &games[ngames++]; // Add game to player's list
    printf("DEBUG: New game with ID %d created by player %s.\n", new_game.id, host->username);
    return new_game.id;
}

void start_game(int fd, struct player *guest, int game_index) {
    struct game *game = &games[game_index];
    game->guest = guest;
    game->status = IN_PROGRESS; // Set the game status to in progress
    lock_games(game->host); // Lock all other games for the host
    lock_games(guest); // Lock all other games for the guest
    printf("DEBUG: Game with ID %d started between %s and %s.\n", game->id, game->host->username, guest->username);
    //TODO: Implement game logic here
}

void lock_games(struct player *player) {
    for (int i = 0; i < player->ngames; i++) {
        if (player->games[i] != NULL) {
            player->games[i]->status = LOCKED; // Lock the game
        }
    }
    printf("DEBUG: All games for player %s have been locked.\n", player->username);
}
