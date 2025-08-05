#include "communication.h"

bool send_response(int fd, char *message, enum StatusCode status_code) {
    struct packet pkt;
    pkt.status_code = htonl(status_code);
    strncpy(pkt.message, message, MAX_MSG_LEN - 1);
    pkt.message[MAX_MSG_LEN - 1] = '\0'; // Ensure null termination
    if (send(fd, &pkt, sizeof(pkt), 0) < 0) {
        perror("send failed");
        close(fd);
        return false;
    }
    return true;
}

bool send_data(void *data, size_t size, int fd) {
    ssize_t bytes_sent = send(fd, data, size, 0);
    if (bytes_sent < 0) {
        perror("send failed");
        close(fd);
        return false;
    }
    return true;
}

__attribute__((overloadable)) bool send(int *integer, int fd) {
    ssize_t bytes_sent = send(fd, &integer, sizeof(int), 0);
    if (bytes_sent < 0) {
        perror("send failed");
        close(fd);
        return false;
    }
    return true;
}

__attribute((overloadable)) bool send(char *string, int fd) {
    ssize_t bytes_sent = send(fd, string, strlen(string), 0);
    if (bytes_sent < 0) {
        perror("send failed");
        close(fd);
        return false;
    }
    return true;
}

void handle_request(int fd, enum Requests request) {
    switch (request) {
        case NEWGAME:
            // Handle new game request
            printf("DEBUG: Handling new game request.\n");
            // Implementation for new game request
            if (ngames < MAX_GAMES) {
                struct player *host = player_get(fd);
				// Check if the host player exists and has space for a new game
                if (host && host->ngames < MAX_GAMES_PER_PLAYER) {
                    //int new_game_id = create_game(fd);
                    struct game new_game;
                    new_game.id = random_id(fd);
                    new_game.host = host; // Get the player from the table
                    new_game.guest = NULL; // Initially no guest
                    games[ngames] = new_game; // Add to the games array
                    host->games[host->ngames++] = &games[ngames++]; // Add game to player's list
                    printf("DEBUG: New game created with ID %d by player %s.\n", new_game.id, new_game.host->username);
                    // Send confirmation response providing the new game ID
					char msg[64];
					snprintf(msg, sizeof(msg), "%u", new_game.id);
					send_response(fd, msg, OK);
					//TODO: Implement waiting for a guest to join the game
				 } else {
					fprintf(stderr, "DEBUG: Player does not exist or has reached max games.\n");
					send_response(fd, "Cannot create new game", ERROR);
				}
            } else {
                // Send error response packet
                send_response(fd, "Max number of games reached", ERROR);
            }
            break;
        case JOINGAME:
            // Handle join game request
            printf("DEBUG: Handling join game request.\n");
            if (ngames > 0) {
				char list_msg[MAX_MSG_LEN];
				struct player *guest = player_get(fd);
                // Get list of games available to join (games without guests and not hosted by the guest)
				strncpy(list_msg, get_game_list(guest), MAX_MSG_LEN - 1);
				if (strlen(list_msg) == 0) {
					strncpy(list_msg, "No active games available", MAX_MSG_LEN - 1);
					send_response(fd, list_msg, ERROR);
					break;
				}
                // All good send confirmation response with the list of games
				send_response(fd, list_msg, OK);
                // Wait for guest to select a game
                int game_id;
                ssize_t bytes_received = recv(fd, &game_id, sizeof(game_id), 0);
                if (bytes_received <= 0) {
                    perror("recv failed or connection closed");
                    close(fd);
                    return;
                }
                game_id = ntohl(game_id); // Convert to host byte order
                printf("DEBUG: Player %s requested to join game ID %d.\n", guest->username, game_id);
                int game_index = get_game_index(game_id);
                // Ask host approval for the join request
                struct player *host = games[game_index].host;
                char buffer[MAX_MSG_LEN];
                snprintf(buffer, sizeof(buffer), "%d;%s", game_id, guest->username);
                if (!send_data(buffer, sizeof(buffer), host->fd)) {
                    fprintf(stderr, "DEBUG: Failed to notify host.\n");
                    return;
                }
                // Wait for host's response
                char response[1];
                bytes_received = recv(host->fd, response, sizeof(response), 0);
                if (bytes_received <= 0) {
                    perror("recv failed or connection closed");
                    close(host->fd);
                    return;
                }
                if (response[0] == 'y' || response[0] == 'Y') {
                    // Host accepted the join request
                    games[game_index].guest = guest; // Set the guest in the game
                    guest->games[guest->ngames++] = &games[game_index]; // Add game to guest's list
                    printf("DEBUG: Player %s joined game with ID %d hosted by %s.\n", guest->username, game_id, host->username);
                    send_response(fd, "Joined game successfully", OK);
                    //TODO: Implement start game logic
                } else {
                    // Host denied the join request
                    printf("DEBUG: Player %s denied join request for game ID %d.\n", host->username, game_id);
                    send_response(fd, "Join request denied by host", DENIED);
                }
            } else {
                // Send error response packet
                send_response(fd, "No active games to join", ERROR);
            }
            break;
        case REMATCH:
            // Handle rematch request
            printf("DEBUG: Handling rematch request.\n");
            // Implementation for rematch request
            break;
        case LOGOUT:
            // Handle logout request
            printf("DEBUG: Handling logout request.\n");
            // Implementation for logout request
            break;
        default:
            fprintf(stderr, "DEBUG: Unknown request received.\n");
            break;
	}
}