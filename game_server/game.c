#include "game.h"

/// @brief Retrieves a game by its ID.
/// @param game_id The ID of the game to retrieve.
/// @return A pointer to the game structure if found, NULL otherwise.
struct game *get_game(int game_id) {
    for (int i = 0; i < ngames; i++) {
        if (games[i].id == game_id) {
            return &games[i];
        }
    }
    return NULL;
}

/// @brief Creates a JSON list of all games for a player.
/// @param player The player whose games should be listed.
/// @return A JSON object containing the list of games.
cJSON *create_game_list(struct player *player) {
    cJSON *list = cJSON_CreateObject();
    cJSON *array = cJSON_AddArrayToObject(list, "games_list");
    for (int i = 0; i < ngames; i++) {
        if (games[i].guest == NULL && games[i].host != player) {
            cJSON *game = cJSON_CreateObject();
            cJSON_AddNumberToObject(game, "game_id", games[i].id);
            cJSON_AddStringToObject(game, "host", games[i].host->username);
            cJSON_AddItemToArray(array, game);
        }
    }
    return list;
}

/// @brief Creates a new game.
/// @param fd The file descriptor of the host player.
/// @param host The player who is hosting the game.
/// @return The ID of the newly created game.
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

// TODO

/// @brief Starts a game between two players.
/// @param fd The file descriptor of the host player.
/// @param guest The player who is joining the game.
/// @param game_index The index of the game to start.
void start_game(int fd, struct player *guest, int game_index) {
    struct game *game = &games[game_index];
    game->guest = guest;
    game->status = IN_PROGRESS; // Set the game status to in progress
    lock_games(game->host); // Lock all other games for the host
    lock_games(guest); // Lock all other games for the guest
    printf("DEBUG: Game with ID %d started between %s and %s.\n", game->id, game->host->username, guest->username);
    //TODO: Implement game logic here
}

/// @brief Locks all games for a player.
/// @param player The player whose games should be locked.
void lock_games(struct player *player) {
    for (int i = 0; i < player->ngames; i++) {
        if (player->games[i] != NULL) {
            player->games[i]->status = LOCKED; // Lock the game
        }
    }
    printf("DEBUG: All games for player %s have been locked.\n", player->username);
}
