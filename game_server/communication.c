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

void handle_request(int fd, enum Requests request) {
    switch (request) {
        case NEWGAME:
            // Handle new game request
            printf("DEBUG: Handling new game request.\n");
            // Implementation for new game request
            if (ngames < MAX_GAMES) {
                // Send confirmation packet
                send_response(fd, "New game created successfully", OK);
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
                send_response(fd, "Joined game successfully", OK);
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