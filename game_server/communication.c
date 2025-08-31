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
        char username[17];
        memset(username, 0, sizeof(username));
        strncpy(username, cJSON_GetObjectItem(request->payload, "username")->valuestring, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0'; // Ensure null-termination
        printf("DEBUG: Received username [%s]\n", username);
        if (username_exists(username)) {
            printf("DEBUG: Username [%s] already in use.\n", username);
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "Username already in use. Choose another one!");
        } else {
            printf("DEBUG: Username [%s] is available.\n", username);
            struct player new_player;
            new_player.fd = fd;
            strncpy(new_player.username, username, sizeof(new_player.username) - 1);
            new_player.username[sizeof(new_player.username) - 1] = '\0'; // Ensure null-termination
            new_player.ngames = 0;
            printf("DEBUG: Player %s with file descriptor %d initilized\n", username, fd);
            player_add(new_player);
            fds[nfds - 1].fd = new_player.fd;
            nplayers++;
            printf("DEBUG: Player %s connected. Total players: %d\n", new_player.username, nplayers);
            response.status_code = OK;
            cJSON_AddStringToObject(response.payload, "message", "Login succesful!");
        }
    } else if (strcmp(request->method, "new_game") == 0) {
        printf("DEBUG: Client requested new game creation\n");
        if (ngames < MAX_GAMES) {
            struct player *host = player_get(fd);
            if (host->ngames < MAX_GAMES_PER_PLAYER) {
                int game_id = create_game(fd, host);
                response.status_code = OK;
                cJSON_AddNumberToObject(response.payload, "game_id", game_id);
            } else {
                response.status_code = ERROR;
                cJSON_AddStringToObject(response.payload, "message", "Max number of games available for selected player reached!");
            }
        } else {
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "Max number of games available for the server reached!");
        }
    } else if (strcmp(request->method, "get_games_list") == 0) {
        struct player *guest = player_get(fd);
        cJSON *game_list = create_game_list(guest);
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
        struct player *player = player_get(fd);
        int id = cJSON_GetObjectItem(request->payload, "game_id")->valueint;
        struct game *game = get_game(id);
        if (game && !game->guest && game->status == WAITING_FOR_GUEST) {
            game->guest = player; // Set the guest player
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
        struct game *game = get_game(id_item->valueint);
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
        struct game *game = get_game(id_item->valueint); 
        message_t message;
        message.payload = cJSON_CreateObject();
        message.status_code = ERROR;
        cJSON_AddStringToObject(message.payload, "message", "Host denied your request");
        if (!send_message(game->guest->fd, &message)) { printf("Could not deliver message to guest"); }
        game->guest = NULL;
        game->status = WAITING_FOR_GUEST;
        response.status_code = OK;
        cJSON_AddStringToObject(response.payload, "message", ""); 
    } 
    else if (strcmp(request->method, "make_move") == 0) {
        bool cleanup = false;
        cJSON *id_item = cJSON_GetObjectItem(request->payload, "game_id");
        struct game *game = get_game(id_item->valueint);
        cJSON *move_item = cJSON_GetObjectItem(request->payload, "move");
        int move = move_item->valueint;
        int row = (move - 1) / 3;
        int col = (move - 1) % 3;
        char buffer[16];
        if (move >= 1 && move <= 9 && game->board[row][col] == 'E') {
            game->board[row][col] = game->host_turn ? 'X' : 'O';
            // Check if the game is over
            enum GameStatus state = evaluate_game_state(game->board);
            message_t host_msg;
            message_t guest_msg;
            switch(state) {
                case DRAW:
                    // Send draw notification
                    guest_msg.payload = cJSON_CreateObject();
                    cJSON_AddStringToObject(guest_msg.payload, "game_state", "Game Over");
                    cJSON_AddStringToObject(guest_msg.payload, "result", "It's a draw!");
                    cJSON_AddStringToObject(response.payload, "game_state", "Game Over");
                    cJSON_AddStringToObject(response.payload, "result", "It's a draw!");
                    send_message(game->guest->fd, &guest_msg);
                    cJSON_Delete(guest_msg.payload);
                    cleanup = true;
                break;
                case PLAYER1_WINS:
                    // Send player 1 wins notification
                    guest_msg.payload = cJSON_CreateObject();
                    cJSON_AddStringToObject(guest_msg.payload, "game_state", "Game Over");
                    cJSON_AddStringToObject(guest_msg.payload, "result", "You lost!");
                    cJSON_AddStringToObject(response.payload, "game_state", "Game Over");
                    cJSON_AddStringToObject(response.payload, "result", "You won!");
                    send_message(game->guest->fd, &guest_msg);
                    cJSON_Delete(guest_msg.payload);
                    cleanup = true;
                break;
                case PLAYER2_WINS:
                // Send player 2 wins notification
                    host_msg.payload = cJSON_CreateObject();
                    cJSON_AddStringToObject(host_msg.payload, "game_state", "Game Over");
                    cJSON_AddStringToObject(host_msg.payload, "result", "You won!");
                    cJSON_AddStringToObject(response.payload, "game_state", "Game Over");
                    cJSON_AddStringToObject(response.payload, "result", "You lost!");
                    send_message(game->host->fd, &host_msg);
                    cJSON_Delete(host_msg.payload);
                    cleanup = true;
                break;
                case UNDECIDED:
                    // Game is not over. Send table to other player
                    game->host_turn = !game->host_turn;
                    if (game->host_turn) {
                        // Send update to host
                        printf("DEBUG: Host Turn!\n");
                        host_msg.payload = cJSON_CreateObject();
                        cJSON_AddStringToObject(host_msg.payload, "game_state", "In Progress");
                        cJSON_AddStringToObject(host_msg.payload, "board", print_board(buffer, game->board));
                        send_message(game->host->fd, &host_msg);
                    } else {
                        // Send update to guest
                        printf("DEBUG: Guest Turn!\n");
                        guest_msg.payload = cJSON_CreateObject();
                        cJSON_AddStringToObject(guest_msg.payload, "game_state", "In Progress");
                        cJSON_AddStringToObject(guest_msg.payload, "board", print_board(buffer, game->board));
                        send_message(game->guest->fd, &guest_msg);
                    }
                break;
            }
            response.status_code = OK;
        } else {
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "Invalid move! Try again.");
        }
        if (cleanup) {
            // Game is over, remove game from overall active games and from host's own games
            // Overall game list
            for (int i = 0; i < ngames; i++) {
                if (games[i]->id == id_item->valueint) {
                    free(games[i]);
                    // Shift remaining games left
                    for (int j = i + 1; j < ngames; j++) {
                        games[j - 1] = games[j];
                    }
                    ngames--;
                    break;
                }
            }
            // Host's own games
            for (int i = 0; i < game->host->ngames; i++) {
                if (game->host->games[i] == game) {
                    // Shift remaining games left
                    for (int j = i + 1; j < game->host->ngames; j++) {
                        game->host->games[j - 1] = game->host->games[j];
                    }
                    game->host->ngames--;
                    break;
                }
            }
        }
    }
    // Finally send response
    if (!send_message(fd, &response)) { printf("DEBUG: Could not send message to fd: %d", fd); }
    cJSON_Delete(response.payload);
}

void cleanup_games_for_player(int fd) {
    struct player *player = player_get(fd);
    for (int i = 0; i < ngames; i++) {
        if (games[i]->host == player && games[i]->guest != NULL) {
            // Host disconnected - notify guest and remove game
            message_t msg;
            msg.status_code = ERROR;
            msg.payload = cJSON_CreateObject();
            cJSON_AddStringToObject(msg.payload, "message", "The host has disconnected.");
            send_message(games[i]->guest->fd, &msg);
            cJSON_Delete(msg.payload);
            free(games[i]);
            // Remove the game by shifting
            for (int j = i + 1; j < ngames; j++) {
                games[j - 1] = games[j];
            }
            ngames--;
            i--; // Adjust i since we removed an element
        } else if (games[i]->guest == player) {
            // Guest disconnected - just mark as NULL, don't remove the game
            games[i]->guest = NULL;
        }
    }
}