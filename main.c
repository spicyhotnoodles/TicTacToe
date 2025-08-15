//TODO: Everytime the server is stopped all clients need to be disconnected.
//TODO: Find a better data structure to store players and games.

#include "game_server/config.h" // Include the configuration header for constants
#include "game_server/types.h" // Include the types header for structures
#include "game_server/communication.h" // Include the communication header for function prototypes
#include "game_server/hash.h" // Include the hash header for player management functions

struct player_table_entry player_table[PLAYER_TABLE_SIZE];
struct pollfd fds[MAX_PLAYERS + 1]; // +1 for the server
struct game games[MAX_GAMES]; // Array to hold active games

int nfds = 0;
int nplayers = 0;
int ngames = 0;

char welcome_msg[MAX_MSG_LEN] = "Welcome to the server!";
char error_msg[MAX_MSG_LEN] = "Cannot resolve request: max number of players reached!";

int main() {

    int server_fd, client_fd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;

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
            message_t message;
            cJSON *tmp = cJSON_CreateObject();
            bool outcome;
            if (nplayers < MAX_PLAYERS) {
                printf("DEBUG: Connection accepted from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                cJSON_AddStringToObject(tmp, "message", welcome_msg);
                message.status_code = OK;
                message.payload = tmp;
                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                nfds++;
                outcome = send_message(client_fd, &message);
            } else {
                fprintf(stderr, "DEBUG: Max players reached. Connection refused.\n");
                cJSON_AddStringToObject(tmp, "message", error_msg);
                message.status_code = ERROR;
                message.payload = tmp;
                outcome = send_message(client_fd, &message);
                close(client_fd);
            }
            cJSON_Delete(message.payload);
            if (!outcome) {
                printf("DEBUG: Failed to send welcome message to client %d\n", client_fd);
                continue;
            } else {
                printf("DEBUG: Welcome message sent to client %d\n", client_fd);
            }
        }
        // Check for data from clients
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                int fd = fds[i].fd;
                message_t *message = receive_message(fd);
                if (message) {
                    handle_request(fd, message);
                } else {
                    printf("DEBUG: client with fd %d has disconnected\n", fd);
                    // Remove client from pollfd array
                    close(fd);
                    for (int j = i; j < nfds - 1; j++) {
                        fds[j] = fds[j + 1];
                    }
                    nfds--;
                    // Remove player from player_table
                    player_remove(fd);
                    nplayers--;
                    // Optionally, clean up any games involving this player
                    // cleanup_games_for_player(fd);
                    // Adjust loop index since we shifted fds
                    i--;
                }
            }
        }
    }
    close(client_fd);
    close(server_fd);
    return 0;
}