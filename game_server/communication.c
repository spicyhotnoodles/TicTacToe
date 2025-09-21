#include "communication.h"

/// @brief Attempts to serialize a message_t object and send it over a file descriptor `fd` while also handling memory cleanup for the serialized data.
/// @param fd The file descriptor to which send the message
/// @param message The message to be sent
/// @return true on success and false if serialization fails, sending fails, or an error occurs. 
bool send_message(int fd, message_t *message) {
    char *serialized = message_serialize(message);
    if (!serialized) {
        fprintf(stderr, "Error! message_serialize failed.\n");
        return false;
    }
    if (send(fd, serialized, strlen(serialized), 0) > 0) { return true; }
    return false;
}

/// @brief Reads data from a file descriptor `fd` using the `recv` system call, processes the received data as a JSON string, and parses it into a `message_t` structure.
/// @param fd The file descriptor to receive the message from
/// @return A `message_t` object if receiving and parsing were successfull, `NULL` otherwise.
message_t * receive_message(int fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes = recv(fd, buffer, BUFFER_SIZE-1, 0);
    if (bytes <= 0) {
        if (bytes == 0) {
            printf("Client disconnected\n");
        } else {
            perror("Failed recv");
        }
        close(fd);
        return NULL;
    }
    buffer[bytes] = '\0';
    // Parse JSON in message_t struct
    message_t *msg = message_parse(buffer);
    if (!msg) {
        fprintf(stderr, "Invalid JSON from fd %d: %s\n", fd, buffer);
        return NULL;
    }
    return msg;
}

/// @brief Processes client requests based on the specified method in the request object. It constructs appropriate responses, manages server and player state, and ensures proper memory handling and communication with clients.
/// @param fd The file descriptor of the requesting client
/// @param request The parsed message from the client
void handle_request(int fd, message_t *request) {
    message_t response;
    response.payload = cJSON_CreateObject(); // Free memory after sending message!
    if (strcmp(request->method, "login") == 0) {
        printf("DEBUG: Client requested login\n");
        char *username = strdup(cJSON_GetObjectItem(request->payload, "username")->valuestring);
        printf("DEBUG: Received username [%s]\n", username);
        if (username_exists(username)) {
            printf("DEBUG: Username [%s] already in use.\n", username);
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "Username already in use. Choose another one!");
        } else {
            printf("DEBUG: Username [%s] is available.\n", username);
            struct player *player = malloc(sizeof(struct player));
            *player = (struct player) {
                .username = strdup(username),
                .fd = fd,
                .hosted_game_ids = NULL,
                .joined_game_ids = NULL
            };
            printf("DEBUG: Player %s with file descriptor %d initialized\n", username, fd);
            g_hash_table_insert(players, GINT_TO_POINTER(player->fd), player);
            printf("DEBUG: Player %s connected. Total players: %d\n", player->username, g_hash_table_size(players));
            response.status_code = OK;
            cJSON_AddStringToObject(response.payload, "message", "Login succesful!");
        }
    } else if (strcmp(request->method, "new_game") == 0) {
        printf("DEBUG: Client requested new game creation\n");
        struct player *host = g_hash_table_lookup(players, GINT_TO_POINTER(fd));
        int game_id = create_game(fd, host);
        response.status_code = OK;
        cJSON_AddNumberToObject(response.payload, "game_id", game_id);
    } else if (strcmp(request->method, "get_games_list") == 0) {
        struct player *player = g_hash_table_lookup(players, GINT_TO_POINTER(fd));
        cJSON *game_list = create_game_list(player);
        cJSON *array = cJSON_GetObjectItem(game_list, "games_list");
        int count = cJSON_GetArraySize(array);
        if (count == 0) {
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "No active games available");
            cJSON_Delete(game_list);
        } else {
            response.status_code = OK;
            response.payload = game_list;
        }
    } else if (strcmp(request->method, "send_join_request") == 0) {
        printf("Got join game request\n");
        struct player *player = g_hash_table_lookup(players, GINT_TO_POINTER(fd));
        int id = cJSON_GetObjectItem(request->payload, "game_id")->valueint;
        struct game *game = g_hash_table_lookup(games, GINT_TO_POINTER(id));
        if (game) {
            game->guest = player; // Set the guest player
            player->joined_game_ids = g_list_insert(player->joined_game_ids, GINT_TO_POINTER(game->id), g_list_length(player->joined_game_ids));
            game->status = WAITING_FOR_HOST; // Update the game status
            struct player *host = game->host;
            message_t notification;
            notification.payload = cJSON_CreateObject();
            notification.status_code = OK;
            cJSON *info = cJSON_CreateObject();
            cJSON_AddStringToObject(info, "user", player->username);
            cJSON_AddNumberToObject(info, "game_id", id);
            cJSON_AddItemToObject(notification.payload, "notification", info);
            if (send_message(host->fd, &notification)) {
                response.status_code = OK;
                cJSON_AddStringToObject(response.payload, "message", "Request successfully delivered! Wait for host approval...");
            } else {
                response.status_code = ERROR;
                cJSON_AddStringToObject(response.payload, "message", "Cannot complete request: cannot deliver message to host");
            }
            cJSON_Delete(notification.payload);
        } else {
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "Cannot complete request: game is unavailable!");
        }
    } else if (strcmp(request->method, "start_game") == 0) {
        printf("Client requested to start game\n");
        cJSON *id_item = cJSON_GetObjectItem(request->payload, "game_id");
        struct game *game = g_hash_table_lookup(games, GINT_TO_POINTER(id_item->valueint));
        // Check if guest has disconnected in the meanwhile
        if (game->guest) {
            game->status = IN_PROGRESS;
            memset(game->board, 'E', sizeof(game->board)); // Init table
            game->host_turn = true;
            cJSON *game_info = cJSON_CreateObject();
            cJSON_AddStringToObject(game_info, "game_state", "In Progress");
            char buffer[16];
            cJSON_AddStringToObject(game_info, "board", print_board(buffer, game->board));
            response.status_code = OK;
            response.payload = game_info;
            message_t message;
            message.payload = cJSON_CreateObject();
            message.status_code = OK;
            if (!send_message(game->guest->fd, &message)) { printf("Could not deliver message to guest"); }
        } else {
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "Cannot start game: guest has disconnected");
        }
    } else if (strcmp(request->method, "send_join_rejection") == 0) {
        cJSON *id_item = cJSON_GetObjectItem(request->payload, "game_id");
        struct game *game = g_hash_table_lookup(games, GINT_TO_POINTER(id_item->valueint));
        message_t message;
        message.payload = cJSON_CreateObject();
        message.status_code = ERROR;
        cJSON_AddStringToObject(message.payload, "message", "Host denied your request");
        if (!send_message(game->guest->fd, &message)) { printf("Could not deliver message to guest"); }
        game->guest->joined_game_ids = g_list_remove(game->guest->joined_game_ids, GINT_TO_POINTER(game->id));
        game->guest = NULL;
        game->status = WAITING_FOR_GUEST;
        response.status_code = OK;
        cJSON_AddStringToObject(response.payload, "message", ""); 
    } 
    else if (strcmp(request->method, "make_move") == 0) {
        bool cleanup = false;
        cJSON *id_item = cJSON_GetObjectItem(request->payload, "game_id");
        struct game *game = g_hash_table_lookup(games, GINT_TO_POINTER(id_item->valueint));
        if (game && game->host && game->guest) {
            char buffer[16];
            cJSON *move_item = cJSON_GetObjectItem(request->payload, "move");
            int move = move_item->valueint;
            int row = (move - 1) / 3;
            int col = (move - 1) % 3;
            if (move >= 1 && move <= 9 && game->board[row][col] == 'E') {
                game->board[row][col] = game->host_turn ? 'X' : 'O';
                // Check if the game is over
                enum GameStatus state = evaluate_game_state(game->board);
                message_t other_player_msg;
                switch(state) {
                    case DRAW:
                        // Send draw notification to both players
                        response.status_code = OK;
                        cJSON_AddStringToObject(response.payload, "game_state", "Game Over");
                        cJSON_AddStringToObject(response.payload, "result", "It's a draw!");
                        other_player_msg.payload = cJSON_CreateObject();
                        other_player_msg.status_code = OK;
                        cJSON_AddStringToObject(other_player_msg.payload, "game_state", "Game Over");
                        cJSON_AddStringToObject(other_player_msg.payload, "result", "It's a draw!");
                        // Check who made the last move
                        if (game->host_turn) {
                            send_message(game->guest->fd, &other_player_msg);
                        } else {
                            send_message(game->host->fd, &other_player_msg);
                        }
                        cleanup = true;
                    break;
                    case PLAYER1_WINS: // X wins - host is X
                        response.status_code = OK;
                        cJSON_AddStringToObject(response.payload, "game_state", "Game Over");
                        cJSON_AddStringToObject(response.payload, "result", game->host_turn ? "You won!" : "You lost!");
                        other_player_msg.payload = cJSON_CreateObject();
                        other_player_msg.status_code = OK;
                        cJSON_AddStringToObject(other_player_msg.payload, "game_state", "Game Over");
                        cJSON_AddStringToObject(other_player_msg.payload, "result", game->host_turn ? "You lost!" : "You won!");
                        // Check who made the last move
                        if (game->host_turn) {
                            send_message(game->guest->fd, &other_player_msg);
                        } else {
                            send_message(game->host->fd, &other_player_msg);
                        }
                        cleanup = true;
                    break;
                    case PLAYER2_WINS: // O wins - guest is O
                        // Host loses, guest wins
                        response.status_code = OK;
                        cJSON_AddStringToObject(response.payload, "game_state", "Game Over");
                        cJSON_AddStringToObject(response.payload, "result", game->host_turn ? "You lost!" : "You won!");
                        other_player_msg.payload = cJSON_CreateObject();
                        other_player_msg.status_code = OK;
                        cJSON_AddStringToObject(other_player_msg.payload, "game_state", "Game Over");
                        cJSON_AddStringToObject(other_player_msg.payload, "result", game->host_turn ? "You won!" : "You lost!");
                        // Check who made the last move
                        if (game->host_turn) {
                            send_message(game->guest->fd, &other_player_msg);
                        } else {
                            send_message(game->host->fd, &other_player_msg);
                        }
                        cleanup = true;
                    break;
                    case UNDECIDED: // Game is not over. Send table to other player
                        game->host_turn = !game->host_turn; // Switch turn using NOT operator
                        other_player_msg.payload = cJSON_CreateObject();
                        other_player_msg.status_code = OK;
                        cJSON_AddStringToObject(other_player_msg.payload, "game_state", "In Progress");
                        cJSON_AddStringToObject(other_player_msg.payload, "board", print_board(buffer, game->board));
                        printf("DEBUG: %s Turn!\n", game->host_turn ? "Host" : "Guest");
                        send_message(game->host_turn ? game->host->fd : game->guest->fd, &other_player_msg);
                    break;
                }
                response.status_code = OK;
            } else {
                response.status_code = ERROR;
                cJSON_AddStringToObject(response.payload, "message", "Invalid move! Try again.");
            }
            if (cleanup) {
                printf("DEBUG: Cleaning up game resources\n");

                struct player *host = game->host;
                struct player *guest = game->guest;

                // Remove game from table
                // Remove game from host's list
                host->hosted_game_ids = g_list_remove(host->hosted_game_ids, GINT_TO_POINTER(game->id));
                // Remove game from guest's list
                guest->joined_game_ids = g_list_remove(guest->joined_game_ids, GINT_TO_POINTER(game->id));
                
                g_hash_table_remove(games, GINT_TO_POINTER(game->id));
            }
        } else {
            cJSON_Delete(response.payload);
            return;
        }
    }
    // Finally send response
    if (!send_message(fd, &response)) { printf("DEBUG: Could not send message to fd: %d", fd); }
    cJSON_Delete(response.payload);
}

void cleanup_games_for_player(int fd) {
    struct player *p = g_hash_table_lookup(players, GINT_TO_POINTER(fd));
    struct game *game;
    // Loop over the list and grab each id to lookup in the table then remove it
    printf("DEBUG: Cleaning up %d hosted games for player %s\n", g_list_length(p->hosted_game_ids), p->username);
    for (GList *l = p->hosted_game_ids; l != NULL; l = l->next) {
        int game_id = GPOINTER_TO_INT(l->data);
        game = g_hash_table_lookup(games, GINT_TO_POINTER(game_id));
        if (!game) { continue; }
        if (game->guest) {
            // Notify guest that host has disconnected
            message_t msg;
            msg.payload = cJSON_CreateObject();
            msg.status_code = ERROR;
            cJSON_AddStringToObject(msg.payload, "message", "Host has disconnected.");
            if (!send_message(game->guest->fd, &msg)) { printf("Could not deliver message to guest"); }
            cJSON_Delete(msg.payload);
        }
        printf("DEBUG: game with ID %d has %s\n", game_id, game->guest ? "a guest" : "no guest");
        g_hash_table_remove(games, GINT_TO_POINTER(game_id));
    }
    printf("DEBUG: Cleaning up %d joined games for player %s\n", g_list_length(p->joined_game_ids), p->username);
    for (GList *l = p->joined_game_ids; l != NULL; l = l->next) {
        int game_id = GPOINTER_TO_INT(l->data);
        game = g_hash_table_lookup(games, GINT_TO_POINTER(game_id));
        if (!game) { continue; }
        // Notify host that guest has disconnected
        message_t msg;
        msg.payload = cJSON_CreateObject();
        msg.status_code = ERROR;
        cJSON_AddStringToObject(msg.payload, "message", "Guest has disconnected.");
        cJSON_AddStringToObject(msg.payload, "game_state", "Game Over");
        if (!send_message(game->host->fd, &msg)) { printf("Could not deliver message to host"); }
        cJSON_Delete(msg.payload);
        game->guest = NULL;
        game->status = WAITING_FOR_GUEST;
        memset(game->board, 'E', sizeof(game->board));
        game->host_turn = true;
    }
    g_list_free(p->hosted_game_ids);
    g_list_free(p->joined_game_ids);
    g_hash_table_remove(players, GINT_TO_POINTER(fd));
}