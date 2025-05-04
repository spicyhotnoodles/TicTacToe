#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "utils.h"
#include <string.h>

void createGame(struct game *games, int *ngames, struct player *host) {
    games[*ngames].match_id = rand();       
    games[*ngames].host = host;             
    games[*ngames].guest = NULL;            
    games[*ngames].status = newCreation;
    (*ngames)++;                            
    printf("DEBUG: Game created successfully. Total games: %d\n", *ngames);
}

char* prepareListOfGamesForClient(struct game *games, int ngames) {
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

 void joinGame(struct game *games, int *ngames , int match_id, struct player guest) {
    for (int i = 0; i < *ngames; i++) {
        if (games[i].match_id == match_id) {
            games[i].guest = &guest;
            games[i].status = inProgress;
            break;
        }
    }
}   these functions may or may not exist once we plan the flow of our work*/