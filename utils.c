#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "utils.h"
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <pthread.h>
#include "thread.h"

void startGame(game* match) {
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, gameThread, (void*)match) != 0) {
        perror("Failed to create game thread");
        return;
    }
    pthread_detach(thread_id);
}

void createGame(game *games, int *ngames, struct player *host) {
    games[*ngames].match_id = rand();       
    games[*ngames].host = host;             
    games[*ngames].guest = NULL;            
    games[*ngames].status = newCreation;
    (*ngames)++;                            
    printf("DEBUG: Game created successfully. Total games: %d\n", *ngames);
}

char* prepareListOfGamesForClient(game *games, int ngames) {
    char *buffer = malloc(4096);
    if (!buffer) {
        perror("Malloc fallita");
        exit(EXIT_FAILURE);
    }

    buffer[0] = '\0';

    for (int i = 0; i < ngames; i++) {
        if (games[i].status == inProgress) {
            continue;
        }
        sprintf(buffer + strlen(buffer), "Game ID: %d | Host: %s ", games[i].match_id, games[i].host->username);
    }

    return buffer;
    
}

int getGameIndexById(game *games, int ngames, int match_id) {
    for (int i = 0; i < ngames; i++) {
        if (games[i].match_id == match_id) {
            return i;
        }
    }
    return -1;
}

bool declineOrAcceptGuest(game *games, int *ngames , int match_id, struct player *guest){
      int index = getGameIndexById(games, *ngames, match_id);
    if (index == -1) {
        fprintf(stderr, "DEBUG: Game with ID %d not found\n", match_id);
        return false;
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "Player %s wants to join your game. Do you accept? (y/n): ", guest->username);

    if (send(games[index].host->fd, buffer, strlen(buffer), 0) < 0) {
        perror("Send failed");
        return false;
    }

    ssize_t bytes_received = recv(games[index].host->fd, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Recv failed");
        return false; 
    }

    if(buffer[0] == 'y' || buffer[0] == 'Y') {
        snprintf(buffer, sizeof(buffer), "The host accepted your request to join the game.\n");
        if (send(guest->fd, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            return false;
        }
        games[index].guest = guest;
        games[index].status = inProgress;
        return true;
    } else if (buffer[0] == 'n' || buffer[0] == 'N') {
        snprintf(buffer, sizeof(buffer), "The host declined your request to join the game.\n");
        if (send(guest->fd, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            return false;
        }
        return false;
    } else {
        snprintf(buffer, sizeof(buffer), "Invalid response. The guest will be denied access.\n");
        if (send(games[index].host->fd, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            return false;
        }
        return false;
    }
}
//TODO joinGame function should start the game or lead to a function that does it
void joinGame(game *games, int *ngames, int match_id, struct player *guest) {
     int index = getGameIndexById(games, *ngames, match_id);
    if (index == -1) {
        fprintf(stderr, "DEBUG: Game with ID %d not found\n", match_id);
        return;
    }

    games[index].guest = guest;
    games[index].status = inProgress;
}

 /*void hostLeavesGame(struct game *games, int *ngames , int match_id) {
    for (int i = 0; i < *ngames; i++) {
        if (games[i].match_id == match_id) {
            games[i].host = games[i].guest;
            games[i].status = waiting;
            games[i].guest = NULL;
            break;
        }
    }
}

void guestLeavesGame(struct game *games, int *ngames , int match_id) {
    for (int i = 0; i < *ngames; i++) {
        if (games[i].match_id == match_id) {
            games[i].status = waiting;
            games[i].guest = NULL;
            break;
        }
    }
}

void setGameStatus(struct game *games, int *ngames , int match_id, gameStatus status) {
    for (int i = 0; i < *ngames; i++) {
        if (games[i].match_id == match_id) {
            games[i].status = status;
            break;
        }
    }
} 
void setGameResult(struct game *games, int *ngames , int match_id, result res) {
    for (int i = 0; i < *ngames; i++) {
        if (games[i].match_id == match_id) {
            games[i].res = res;
            break;
        }
    }
} 

s   these functions may or may not exist once we plan the flow of our work*/