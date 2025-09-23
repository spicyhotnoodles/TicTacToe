#include "game_server/config.h" // Include the configuration header for constants
#include "game_server/types.h" // Include the types header for structures
#include "game_server/communication.h" // Include the communication header for function prototypes
#include "game_server/hash.h" // Include the hash header for player management functions
#include "game_server/game.h" // Include the game header for game management functions

GArray *fds; // Dynamic array to hold poll file descriptors
GHashTable *games; // Hash table to store active games
GHashTable *players; // Hash table to store active players

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

    // Init game hash table
    games = g_hash_table_new_full(  // g_hash_table_new_full allows to specify for custom free functions
        g_direct_hash, // hash function for integer keys
        g_direct_equal, // key equality function (keys are integers, use default)
        NULL, // key free function (keys are integers, no need to free)
        free_game // value free function (custom function to free game resources)
    );

    // Init player hash table
    players = g_hash_table_new(
        g_direct_hash, 
        g_direct_equal
    );

    // Initialize dynamic array for poll file descriptors
    fds = g_array_new(FALSE, FALSE, sizeof(struct pollfd));
    struct pollfd server = {
        .fd = server_fd,
        .events = POLLIN
    };
    g_array_append_val(fds, server); // Add server fd to the array

    printf("Server listening on port %d...\n", PORT);

    while (true) {
        if (poll(&g_array_index(fds, struct pollfd, 0),
                fds->len,
                -1) < 0) {
            perror("poll failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        // Check for new connections
        if (g_array_index(fds, struct pollfd, 0).revents & POLLIN) { // g_array_index gets the first element of the array, which is the server fd
            if ((client_fd = accept(server_fd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                perror("accept failed");
                close(server_fd);
                exit(EXIT_FAILURE);
            }
            message_t message;
            message.payload = cJSON_CreateObject();
            message.status_code = OK;
            cJSON_AddStringToObject(message.payload, "message", "Welcome to the server!");
            struct pollfd new_fd = {
                .fd = client_fd,
                .events = POLLIN
            };
            g_array_append_val(fds, new_fd); // Add new client fd to the array
            printf("DEBUG: New client connected with fd %d\n", client_fd);
            // Send welcome message
            if (!send_message(client_fd, &message)) {
                printf("DEBUG: Failed to send welcome message to client %d\n", client_fd);
                close(client_fd);
                g_array_remove_index(fds, fds->len - 1);
            } else {
                printf("DEBUG: Welcome message sent to client %d\n", client_fd);
            }
        }
        // Check for data from clients
        for (guint i = 1; i < fds->len; i++) {
            if (g_array_index(fds, struct pollfd, i).revents & POLLIN) {
                int fd = g_array_index(fds, struct pollfd, i).fd;
                message_t *message = receive_message(fd);
                if (message) {
                    // Process client request
                    handle_request(fd, message);
                } else {
                    printf("DEBUG: client with fd %d has disconnected\n", fd);
                    if (g_hash_table_contains(players, GINT_TO_POINTER(fd))) {
                        cleanup_games_for_player(fd);
                    }
                    // Remove client from pollfd array
                    close(fd);
                    // Shift elements left to fill the gap
                    for (guint j = i; j < fds->len - 1; j++) {
                        g_array_index(fds, struct pollfd, j) = g_array_index(fds, struct pollfd, j + 1);
                    }
                    // Remove last element since it's now a duplicate
                    fds = g_array_remove_index(fds, fds->len - 1);
                    // Adjust loop index since we shifted fds
                    i--;
                }
            }
        }
    }
    free(fds);
    g_hash_table_destroy(games);
    g_hash_table_destroy(players);
    close(client_fd);
    close(server_fd);
    return 0;
}