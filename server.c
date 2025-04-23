#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>


#define PORT 8080
#define QLEN 2
#define MAX_PLAYERS 50
#define MAX_GAMES 25

struct player {
	int fd;
	char username[16];
};

struct game {
	int match_id;
	struct player host;
	struct player *guest; // Is a pointer because it could be NULL
};

struct player players[MAX_PLAYERS];
struct pollfd fds[MAX_PLAYERS + 1]; // +1 for the server
int nfds = 0;
int nplayers = 0;
int ngames = 0;
struct game games[MAX_GAMES];

int main() {
	int server_fd, client_fd;
	struct sockaddr_in address;
	char buffer[1024];
	srand(time(NULL)); // Seed for random number generation
	// Creating socket.
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket failed.");
		exit(EXIT_FAILURE);
	}
	// Set socket options to reuse immediately the PORT
	if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) < 0) {
		perror("Setsockopt failed.");
		exit(1);
	}
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	socklen_t length = sizeof(address);
	// Bind socket to address
	if ((bind(server_fd, (struct sockaddr *) &address, length)) < 0) {
		perror("Bind failed.");
		exit(EXIT_FAILURE);
	}
	// Set socket to listen on PORT
	if (listen(server_fd, QLEN) < 0) {
		perror("Listen failed.");
		exit(EXIT_FAILURE);
	}
	printf("Waiting for connections...\n");
	fds[0].fd = server_fd;
	fds[0].events = POLLIN; // Monitor for incoming connections
	nfds++;
	while (true) {
		// Accept incoming connection
		if (poll(fds, nfds, -1) < 0) { // -1 Wait indefinitely
			perror("Poll failed.");
			exit(EXIT_FAILURE);
		}
		// Check for new connections
		if (fds[0].revents && POLLIN) {
			if ((client_fd = accept(server_fd, (struct sockaddr *) &address, &length)) < 0) {
				perror("Accept failed.");
				exit(EXIT_FAILURE);
			}
			// Add new client to pollfd array
			if (nfds < MAX_PLAYERS) {
				printf("Connection accepted.\n");
				fds[nfds].fd = client_fd;
				fds[nfds].events = POLLIN;
				// Initialize player
				players[nplayers].fd = client_fd;
				ssize_t bytes_read = read(client_fd, buffer, 1024 - 1);
				if (bytes_read < 0) {
					perror("Read failed.");
					exit(EXIT_FAILURE);
				} else if (bytes_read == 0) {
					// Invalid username
					printf("Error: empty username!");
					close(client_fd);
					fds[nfds] = fds[nfds - 1];
					nfds--;
				} else {
					// Set username
					memset(players[nplayers].username, 0, 16);
					strncpy(players[nplayers].username, buffer, 16);
					nfds++;
					nplayers++;
					printf("Username successfully set.\n");
					printf("#DEBUG List of players:\n");
					for (int i = 0; i < nplayers; i++) {
						printf("Player #%d: %s, FileDescriptor: %d\n", i + 1, players[i].username, players[i].fd);
					}
				}
			} else {
				fprintf(stderr, "Max players reached\n");
				close(client_fd);
			}
		}
		// Check client sockets for data
		for (int i = 1; i < nfds; i++) {
		    if (fds[i].revents & POLLIN) {
		        char buffer[1024];
		        ssize_t bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
		        if (bytes <= 0) {
		            // Client disconnected
		            close(fds[i].fd);
		            fds[i] = fds[nfds - 1]; // Replace with last entry
		            players[i] = players[nfds - 1]; // Update client info
		            nfds--;
		            i--; // Recheck current index
		        } else {
					printf("DEBUG: Received message: %s\n", buffer);
					int option = strcmp(buffer, "new_game");
					printf("#DEBUG: option = %d", option);
		            switch(option) {
						case 0: 
							// Create new game
							struct game new_game;
							new_game.match_id = rand() % 1000 + 1; // Random match ID
							new_game.host = players[i - 1]; // Host is the player who created the game
							new_game.guest = NULL; // No guest yet
							games[ngames] = new_game; // Add game to list
							ngames++;
							printf("New game created with ID: %d\n", new_game.match_id);
							// Notify host
							send(fds[i].fd, "Game created successfully", 25, 0);
						 case 1:
							printf("DEBUG: Player %s requested game list\n", players[i - 1].username);
							// Display list of games
							if (ngames == 0) {
								printf("No games available\n");
							} else {
								for (int j = 0; j < ngames; j++) {
									char game_info[50];
									sprintf(game_info, "Game ID: %d, Host: %s\n", games[j].match_id, games[j].host.username);
									send(fds[i].fd, game_info, strlen(game_info), 0);
								}
							}
					}
		        }
		    }
		}
	}
	return 0;
}