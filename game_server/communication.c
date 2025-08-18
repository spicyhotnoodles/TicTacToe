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
        if (!game->guest && game->status == WAITING_FOR_GUEST) {
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
        printf("Got start game request\n");
        if (request->payload) {
            // TODO
        } else {
            fprintf(stderr, "Error! payload item is NULL\n");
            response.status_code = ERROR;
            cJSON_AddStringToObject(response.payload, "message", "Cannot complete request: game ID was not specified!");
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

    }
    // Finally send response
    if (!send_message(fd, &response)) { printf("DEBUG: Could not send message to fd: %d", fd); }
    cJSON_Delete(response.payload);
}