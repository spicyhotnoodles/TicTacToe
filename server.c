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
#include <pthread.h>

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

enum Request {
	NEW_GAME = 0,
	JOIN_GAME = 1,
	LEAVE_GAME = 2
};

enum Response {
	OK = 0,
	ERROR = 1
};

struct player players[MAX_PLAYERS];
struct pollfd fds[MAX_PLAYERS + 1]; // +1 for the server
int nfds = 0;
int nplayers = 0;
int ngames = 0;
struct game games[MAX_GAMES];

// Thread routine to handle game creation logic
void *handle_game(void *args) {
	// For test purposes...
	sleep(4);
	printf("Notifying host game is starting\n");
	int data = htonl(GAME_START);
	int fd = *(int *) args;
	if (send(fd, &data, sizeof(data), 0) < 0) {
		perror("Send failed");
		exit(EXIT_FAILURE);
	}
	pthread_exit(NULL);
}

int main() {
	int server_fd, client_fd;
	struct sockaddr_in address;
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
				char username[17];
				fds[nfds].fd = client_fd;
				fds[nfds].events = POLLIN;
				// Initialize player
				players[nplayers].fd = client_fd;
				ssize_t bytes_read = read(client_fd, username,  sizeof(username));
				if (bytes_read < 0) {
					perror("Read failed.");
					exit(EXIT_FAILURE);
				}  else {
					// Set username
					memset(players[nplayers].username, 0, 16);
					strncpy(players[nplayers].username, username, strlen(username));
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
				int received_data = 0;
		        ssize_t bytes = recv(fds[i].fd, &received_data, sizeof(received_data), 0);
		        if (bytes < 0) {
		            // Client disconnected
		            close(fds[i].fd);
		            fds[i] = fds[nfds - 1]; // Replace with last entry
		            players[i] = players[nfds - 1]; // Update client info
		            nfds--;
		            i--; // Recheck current index
		        } else if (bytes == 0) {
					printf("Client disconnected.\n");
					close(fds[i].fd);
					fds[i] = fds[nfds - 1]; // Replace with last entry
					players[i] = players[nfds - 1]; // Update client info
					nfds--;
					i--; // Recheck current index
				} else {
					switch (ntohl(received_data)) {
						case NEW_GAME:
							printf("DEBUG: Player %s requested to create a new game\n", players[i - 1].username);
							if (ngames < MAX_GAMES) {
								printf("DEBUG: Request is possible. Sending approval response to host\n");
								int data = htonl(OK);
								if (send(fds[i].fd, &data, sizeof(data), 0) < 0) {
									perror("Send failed");
									exit(EXIT_FAILURE);
								}
								struct game new_game;
								new_game.match_id = rand() % 1000; // Random match ID
								new_game.host = players[i - 1]; // Host is the player who created the game
								new_game.guest = NULL; // No guest yet
								games[ngames] = new_game; // Add game to list
								ngames++;
								printf("DEBUG: Game created with ID %d\n", new_game.match_id);
								// TODO: #0. (Optional) Create a thread to manage logic #1. Create a game #2. Wait for another player #3. Notify host #4. Monitor game   
								pthread_t thread_id;
								pthread_attr_t attr;
								pthread_attr_init(&attr);
								if (pthread_create(&thread_id, &attr, &handle_game, (void *) &fds[i].fd) < 0) {
									perror("Thread creation failed");
									exit(EXIT_FAILURE);
								}
								pthread_join(thread_id, NULL);
							} else {
								printf("DEBUG: Error, request is not possible; Max number of games reached\n");
								int data = htonl(ERROR);
								if (send(fds[i].fd, &data, sizeof(data), 0) < 0) {
									perror("Send failed");
									exit(EXIT_FAILURE);
								}
							}
							break;
						case JOIN_GAME:
							printf("DEBUG: Player %s requested to join a game\n", players[i - 1].username);
							if (ngames < 1) {
								printf("DEBUG: Error, request is not possible; No games available\n");
								int data = htonl(ERROR);
								if (send(fds[i].fd, &data, sizeof(data), 0) < 0) {
									perror("Send failed");
									exit(EXIT_FAILURE);
								}
							} else {
								printf("DEBUG: Request is possible. Sending approval response to host\n");
								int data = htonl(OK);
								if (send(fds[i].fd, &data, sizeof(data), 0) < 0) {
									perror("Send failed");
									exit(EXIT_FAILURE);
								}
							}
							break;
						case LEAVE_GAME:
							printf("DEBUG: Player %s requested to leave a game\n", players[i - 1].username);
							break;
						default:
							printf("DEBUG: Player %s sent an unknown request\n", players[i - 1].username);
							break;
					}
		        }
		    }
		}
	}
	return 0;
}