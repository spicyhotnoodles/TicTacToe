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