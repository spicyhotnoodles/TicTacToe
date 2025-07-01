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
            // Implementation for join game request
            if (ngames > 0) {
                // Send confirmation packet
				char list_msg[MAX_MSG_LEN];
				struct player *player = player_get(fd);
				for (int i = 0; i < ngames; i++) {
					if (games[i].guest == NULL && games[i].host != player) { // Only list games without guests
						char game_info[64];
						snprintf(game_info, sizeof(game_info), "Game ID: %d, Host: %s\n", games[i].id, games[i].host->username);
						strncat(list_msg, game_info, sizeof(list_msg) - strlen(list_msg) - 1);
					}
				}
				// list_msg[MAX_MSG_LEN - 1] = '\0'; // Ensure null termination
				if (strlen(list_msg) == 0) {
					strncpy(list_msg, "No active games available", MAX_MSG_LEN - 1);
					send_response(fd, list_msg, ERROR);
					break;
				}
				send_response(fd, list_msg, OK);
				//TODO: Implement wait for host to approve the join request
				break;
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