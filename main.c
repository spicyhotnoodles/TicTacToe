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

    int request;

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
                if (!send_response(client_fd, welcome_msg, OK)) {
                    continue; // If sending the welcome message fails, skip to the next iteration
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
                         if (username_exists(username)) {
                            printf("DEBUG: Username [%s] already exists.\n", username);
                            // Send error response with packet
                           if (!send_response(client_fd, "Username already exists", ERROR)) {
                                break; // If sending the error message fails, break out of the loop
                           }
                        } else {
                            // Initialize player
                            struct player new_player;
                            new_player.fd = client_fd;
                            strncpy(new_player.username, username, sizeof(new_player.username) - 1);
                            new_player.username[sizeof(new_player.username) - 1] = '\0'; // Ensure null termination
                            new_player.ngames = 0; // Initialize number of games
                            printf("DEBUG: Player initialized: %s\n", new_player.username);
                            // Add player to the list
                            player_add(new_player);
                            fds[nfds - 1].fd = new_player.fd; // Update the fd
                            nplayers++;
                            printf("DEBUG: Player %s connected. Total players: %d\n", new_player.username, nplayers);
                            // Send welcome message
                            send_response(client_fd, welcome_msg, OK); 
                            break;
                        }
                    }
                }
            } else {
                fprintf(stderr, "DEBUG: Max players reached. Connection refused.\n");
                // Send error response packet
                send_response(client_fd, error_msg, ERROR);
                close(client_fd);
                continue;
            }
        } 

        // Check for data from clients
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                ssize_t bytes_received = recv(fds[i].fd, &request, sizeof(request), 0);
                if (bytes_received <= 0) {
                    // Client disconnected or error
                    perror("recv failed or connection closed");
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1]; // Replace with last entry
                    nfds--;
                    nplayers--;
                    continue;
                }
                printf("DEBUG: Received request %d from player with fd %d\n", ntohl(request), fds[i].fd);
                handle_request(fds[i].fd, ntohl(request)); // Handle the request
            }
        } 

    }
    close(client_fd);
    close(server_fd);
    return 0;
}