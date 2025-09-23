#include "game.h"

/// @brief Creates a JSON list of all games for a player.
/// @param player The player whose games should be listed.
/// @return A JSON object containing the list of games.
cJSON *create_game_list(struct player *player) {
    cJSON *list = cJSON_CreateObject();
    cJSON *array = cJSON_AddArrayToObject(list, "games_list");
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, games);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        struct game *game = (struct game *)value;
        if (game->host != player && game->status == WAITING_FOR_GUEST) {
            cJSON *game_info = cJSON_CreateObject();
            cJSON_AddNumberToObject(game_info, "game_id", game->id);
            cJSON_AddStringToObject(game_info, "host", game->host->username);
            cJSON_AddItemToArray(array, game_info);
        }
    }
    return list;
}

/// @brief Creates a new game.
/// @param fd The file descriptor of the host player.
/// @param host The player who is hosting the game.
/// @return The ID of the newly created game.
int create_game(int fd, struct player *host) {
    struct game *new_game = malloc(sizeof(struct game));
    int game_id;
    do {
        game_id = random_id(fd);
    } while (g_hash_table_lookup(games, GINT_TO_POINTER(game_id)) != NULL);
    *new_game = (struct game) {
        .id = game_id,
        .host = host,
        .guest = NULL,
        .status = WAITING_FOR_GUEST
    };
    g_hash_table_insert(games, GINT_TO_POINTER(new_game->id), new_game);
    host->hosted_game_ids = g_list_insert(host->hosted_game_ids, GINT_TO_POINTER(new_game->id), g_list_length(host->hosted_game_ids));
    printf("DEBUG: New game with ID %d created by player %s.\n", new_game->id, host->username);
    return new_game->id;
}

/// @brief Evaluates the current state of the game.
/// @param board The game board.
/// @return The status of the game.
enum GameStatus evaluate_game_state(char board[][3]) {
    char potential_winner = '\0';
    // Check for horizontal win
    for (int i = 0; i < 3; i++) {
        potential_winner = board[i][0];
        if (potential_winner != 'E') {
            if (board[i][0] == board[i][1] && board[i][0] == board[i][2]) {
                goto who_won;
            }
        }
    }
    // Check for vertical win
    for (int i = 0; i < 3; i++) {
        potential_winner = board[0][i];
        if (potential_winner != 'E') {
            if (board[0][i] == board[1][i] && board[0][i] == board[2][i]) {
                goto who_won;
            }
        }
    }
    // Check for left diagonal win
    potential_winner = board[0][0];
    if (potential_winner != 'E') {
        if (board[0][0] == board[1][1] && board[0][0] == board[2][2]) {
            goto who_won;
        }
    }
    // Check for right diagonal win
    potential_winner = board[0][2];
    if (potential_winner != 'E') {
        if (board[0][2] == board[1][1] && board[0][2] == board[2][0]) {
            goto who_won;
        }
    }
    // Check if the board is full
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == 'E') {
                return UNDECIDED;
            }
        }
    }
    return DRAW;
    who_won:
        return (potential_winner == 'X' ? PLAYER1_WINS : PLAYER2_WINS);
}

/// @brief Prints the current state of the game board to specified buffer.
/// @param buffer The buffer to store the printed board.
/// @param board The game board.
/// @return The buffer containing the printed board.
char * print_board(char buffer[], char board[][3]) {
    int k = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == 'E') {
                // Calculate position number (1-9) from row and column
                int position = i * 3 + j + 1;
                buffer[k] = '0' + position;  // Convert position to char '1'-'9'
            } else {
                buffer[k] = board[i][j];
            }
            k++;
        }
        buffer[k] = '\n';  // Add newline after each row
        k++;
    }
    buffer[k] = '\0'; // Null terminate the string
    return buffer;
}

/// @brief Frees the memory allocated for a game.
/// @param data The game data to free.
void free_game(gpointer data) {
    if (!data) {
        return;  // Prevent crash on NULL pointer
    }
    struct game *g = (struct game *)data;
    free(g);
}