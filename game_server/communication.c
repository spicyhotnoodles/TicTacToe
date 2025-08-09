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
                    int game_id = create_game(fd, host);
                    // Send confirmation response providing the new game ID
					char msg[64];
					snprintf(msg, sizeof(msg), "%u", game_id);
					send_response(fd, msg, OK);
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
                // TODO
                /* Next steps:
                 1. Wait for guest to send the game ID they want to join
                 2. Send a notification to the host of the game
                 3. If the host approves, start the game
                 4. If the host denies, send a denial response to the guest */
            } else {
                send_response(fd, "No active games available", ERROR);
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