//TODO: The logic that checks if a username already exists is not working properly. Fix it.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdbool.h>


#define PORT 12345
#define MAX_MSG_LEN 256
#define MAX_PLAYERS 5
#define PLAYER_TABLE_SIZE 32

struct player {
    int fd;
    char username[17]; // 16 characters + null terminator
};

struct player_table_entry {
    int fd;
    struct player data;
    bool in_use;
};

struct player_table_entry player_table[PLAYER_TABLE_SIZE];

int hash_fd(int fd) {
    return fd % PLAYER_TABLE_SIZE;
}

bool player_add(struct player p) {
    int index = hash_fd(p.fd);
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        int try = (index + i) % PLAYER_TABLE_SIZE;
        if (!player_table[try].in_use) {
            player_table[try].fd = p.fd;
            player_table[try].data = p;
            player_table[try].in_use = true;
            return true;
        }
    }
    return false; // Table full
}

struct player* player_get(int fd) {
    int index = hash_fd(fd);
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        int try = (index + i) % PLAYER_TABLE_SIZE;
        if (player_table[try].in_use && player_table[try].fd == fd) {
            return &player_table[try].data;
        }
    }
    return NULL;
}

bool player_remove(int fd) {
    int index = hash_fd(fd);
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        int try = (index + i) % PLAYER_TABLE_SIZE;
        if (player_table[try].in_use && player_table[try].fd == fd) {
            player_table[try].in_use = false;
            return true;
        }
    }
    return false;
}

bool username_exists(const char* username) {
    for (int i = 0; i < PLAYER_TABLE_SIZE; i++) {
        if (player_table[i].in_use && strcmp(player_table[i].data.username, username) == 0) {
            return true;
        }
    }
    return false;
}

enum Requests {
    NEWGAME,
    JOINGAME,
    REMATCH,
    LOGOUT
};

enum StatusCode {
    OK,
    ERROR,
    DENIED
};

// Ensure structure is tightly packed (no padding)
#pragma pack(push, 1)
struct packet {
    int status_code;
    char message[MAX_MSG_LEN];
};
#pragma pack(pop)

struct pollfd fds[MAX_PLAYERS + 1]; // +1 for the server
int nfds = 0;
int nplayers = 0;
int ngames = 0;

char welcome_msg[MAX_MSG_LEN] = "Welcome to the server!";
char error_msg[MAX_MSG_LEN] = "Cannot resolve request: max number of players reached!";

int main() {

    int server_fd, client_fd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;

    struct packet pkt;
    memset(&pkt, 0, sizeof(pkt));  // Zero-initialize

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse immediately the PORT
    if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) < 0) {
        perror("setsockopt failed.");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    fds[0].fd = server_fd;
    fds[0].events = POLLIN; // Monitor for incoming connections
    nfds++;

    while (true) {
        if (poll(fds, nfds, -1) < 0) { // -1 Wait indefinitely
            perror("poll failed");
            close(client_fd);
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // Check for new connections
        if (fds[0].revents & POLLIN) {
            if ((client_fd = accept(server_fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                perror("accept failed");
                close(server_fd);
                exit(EXIT_FAILURE);
            }
            if (nplayers < MAX_PLAYERS) {
                printf("Connection accepted from %s:%d\n",
                       inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                struct packet accept_response;
                accept_response.status_code = htonl(OK);
                strncpy(accept_response.message, "Access granted", MAX_MSG_LEN - 1);
                accept_response.message[MAX_MSG_LEN - 1] = '\0';

                // Try to send the rejection packet before closing
                if (send(client_fd, &accept_response, sizeof(accept_response), 0) < 0) {
                    perror("send failed");
                    close(client_fd);
                    continue;
                }
                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                nfds++;
                char username[17];
                printf("DEBUG: Waiting for username input...\n");
                while (true) {
                    memset(username, 0, sizeof(username));
                    ssize_t bytes_read = read(client_fd, username, sizeof(username));
                    printf("Received %zd bytes from client.\n", bytes_read);
                    if (bytes_read < 0) {
                        perror("read failed");
                        close(client_fd);
                    } else {
                        // Check if username is already taken
                        //strncpy(username, username, bytes_read - 1); // Exclude null terminator
                        printf("DEBUG: Username received: %s\n", username);
                        /* if (username_exists(username)) {
                            printf("DEBUG: Username [%s] already exists.\n", username);
                            // Send error response with packet
                            struct packet response;
                            response.status_code = htonl(ERROR);
                            strncpy(response.message, "Username already exists", MAX_MSG_LEN - 1);
                            response.message[MAX_MSG_LEN - 1] = '\0'; // Ensure null termination
                            if (send(client_fd, &response, sizeof(response), 0) < 0) {
                                perror("send failed");
                                close(client_fd);
                                break;
                            }
                        } else { */
                            // Initialize player
                            struct player new_player;
                            new_player.fd = client_fd;
                            strncpy(new_player.username, username, sizeof(new_player.username) - 1);
                            new_player.username[sizeof(new_player.username) - 1] = '\0'; // Ensure null termination
                            printf("DEBUG: Player initialized: %s\n", new_player.username);
                            // Add player to the list
                            fds[nfds - 1].fd = new_player.fd; // Update the fd
                            nplayers++;
                            printf("DEBUG: Player %s connected. Total players: %d\n", new_player.username, nplayers);
                            // Send welcome message
                            struct packet welcome_packet;
                            welcome_packet.status_code = htonl(OK);
                            strncpy(welcome_packet.message, welcome_msg, MAX_MSG_LEN - 1);
                            welcome_packet.message[MAX_MSG_LEN - 1] = '\0'; // Ensure null termination
                            if (send(client_fd, &welcome_packet, sizeof(welcome_packet), 0) < 0) {
                                perror("send failed");
                                close(client_fd);
                            }    
                            break;
                        //}
                    }
                }
            } else {
                fprintf(stderr, "DEBUG: Max players reached. Connection refused.\n");
                // Send error response without packet
                struct packet full_response;
                full_response.status_code = htonl(ERROR);
                strncpy(full_response.message, error_msg, MAX_MSG_LEN - 1);
                full_response.message[MAX_MSG_LEN - 1] = '\0';

                // Try to send the rejection packet before closing
                if (send(client_fd, &full_response, sizeof(full_response), 0) < 0) {
                    perror("send failed");
                }

                close(client_fd);
                continue;
            }
    
            // Check for data from clients
            for (int i = 1; i < nfds; i++) {
                if (fds[i].revents & POLLIN) {
                    ssize_t bytes_received = recv(fds[i].fd, &pkt, sizeof(pkt), 0);
                    if (bytes_received <= 0) {
                        // Client disconnected or error
                        perror("recv failed or connection closed");
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1]; // Replace with last entry
                        nfds--;
                        nplayers--;
                        continue;
                    }

                    pkt.status_code = ntohl(pkt.status_code);
                    printf("DEBUG: Received request from player %s: %d\n", pkt.message, pkt.status_code);

                    switch (pkt.status_code) {
                        case NEWGAME:
                            printf("DEBUG: Player requested to create a new game.\n");
                            // Handle new game request
                            break;
                        case JOINGAME:
                            printf("DEBUG: Player requested to join a game.\n");
                            // Handle join game request
                            break;
                        case REMATCH:
                            printf("DEBUG: Player requested a rematch.\n");
                            // Handle rematch request
                            break;
                        case LOGOUT:
                            printf("DEBUG: Player requested to logout.\n");
                            // Handle logout request
                            player_remove(fds[i].fd);
                            close(fds[i].fd);
                            fds[i] = fds[nfds - 1]; // Replace with last entry
                            nfds--;
                            nplayers--;
                            break;
                        default:
                            fprintf(stderr, "DEBUG: Unknown request received.\n");
                            break;
                    }
                }
            } 
        } 
    }
    close(client_fd);
    close(server_fd);
    return 0;
}